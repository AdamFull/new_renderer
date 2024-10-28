#include <engine/rendering/Rendering.h>

#include <engine/rendering/gfx/Context.h>

#include <engine/rendering/RenderContext.h>
#include <engine/rendering/UploadContext.h>

#include <engine/rendering/render_graph/ComputePass.h>
#include <engine/rendering/render_graph/RenderPass.h>

#include <engine/rendering/GeometryBuffer.h>

#include <util/Logger.h>

#ifdef MemoryBarrier
#undef MemoryBarrier
#endif

namespace engine
{
	namespace render
	{
		bool IsXOnlyAccess(const RenderGraphResource& resource, RenderGraphResourceAccess required_access)
		{
			int32_t access_index = utl::enum_cast(required_access);
			for (int32_t i = 0; i < 3ull; ++i)
			{
				if (i != access_index && !resource.usages[i].empty())
					return false;
			}

			return true;
		}

		bool IsROAccessOnlyRemain(const RenderGraphResource& resource, u64 current_pass)
		{
			for (int32_t i = 0; i < 3ull; ++i)
			{
				for (const auto& pass_index : resource.usages[i])
				{
					if (pass_index > current_pass)
						return false;
				}
			}

			return true;
		}

		gfx::AccessTypeFlag GetAccessFlagType(RenderGraphResourceAccess type)
		{
			gfx::AccessTypeFlag access_flag{};
			switch (type)
			{
			case engine::render::RenderGraphResourceAccess::eRead: access_flag = gfx::AccessTypeFlag::eRead; break;
			case engine::render::RenderGraphResourceAccess::eWrite: access_flag = gfx::AccessTypeFlag::eWrite; break;
			case engine::render::RenderGraphResourceAccess::eReadAndWrite: access_flag = gfx::AccessTypeFlag::eRW; break;
			default: break;
			}

			return access_flag;
		}


		Renderer::Renderer(Window* window, filesystem::IVirtualFileSystemInterface* vfs) :
			vfs{ vfs }
		{
			gfx::ContextCreateInfo gfx_create_info{};
			gfx_create_info.app_name = "MyVulkanEngine";
			gfx_create_info.render_api = gfx::RenderApi::eVulkan;
			gfx_create_info.window = window;

			gfx_context = gfx::CreateContext(gfx_create_info);

			render_context = std::make_unique<RenderContext>(*gfx_context, window);
			resource_context = std::make_unique<UploadContext>(*gfx_context, vfs);

			// Create timestamp query for collecting pass info
			gfx::QueryPoolCreateInfo query_pool_ci{};
			query_pool_ci.type = gfx::QueryType::eTimestamp;
			query_pool_ci.query_count = 2u;
			timestamp_query = gfx_context->CreateQueryPool(query_pool_ci);
			timestamp_query->Reset(0u);

			gfx_context->RequestTimestampFrequency(&timestamp_frequency);

			// Create default image sampler
			gfx::SamplerCreateInfo sampler_ci{};
			sampler_ci.address_u = sampler_ci.address_v = sampler_ci.address_w = gfx::SamplerAddressMode::eClampToEdge;
			sampler_ci.min_filter = gfx::Filter::eLinear;
			sampler_ci.mag_filter = gfx::Filter::eLinear;
			sampler_ci.mip_map_mode = gfx::Filter::eLinear;

			default_sampler = gfx_context->CreateSampler(sampler_ci);

			// Temporary func to create test render graph
			BuildGraph();
		}

		Renderer::~Renderer()
		{
			// Destroy render graph including shader passes
			DestroyRenderGraph();

			gfx_context->Destroy(default_sampler);
			gfx_context->Destroy(timestamp_query);
		}

		uint32_t Renderer::AddTextureResource(const std::string& path, gfx::Sampler* sampler)
		{
			auto& new_resource = graph_resources.emplace_back();
			new_resource.resource_type = RenderResourceTypeFlag::eStaticImage;

			gfx::Texture* texture = resource_context->LoadTexture(path);
			new_resource.resource_id = textures.Add(TextureObject{ texture, sampler ? sampler : default_sampler });

			return graph_resources.size() - 1ull;
		}

		uint32_t Renderer::AddTextureResource(const gfx::TextureCreateInfo& create_info, gfx::Sampler* sampler)
		{
			auto& new_resource = graph_resources.emplace_back();

			// TODO: What if it's storage target and render target at the same time?
			if (create_info.usage & gfx::TextureUsageFlag::eStorage)
				new_resource.resource_type = RenderResourceTypeFlag::eStorageTarget;
			else if (create_info.usage & gfx::TextureUsageFlag::eRenderTarget)
				new_resource.resource_type = RenderResourceTypeFlag::eRenderTarget;
			else
				new_resource.resource_type = RenderResourceTypeFlag::eStaticImage;

			gfx::Texture* texture = gfx_context->CreateTexture(create_info);
			if (create_info.usage & gfx::TextureUsageFlag::eRenderTarget)
			{
				new_resource.resource_id = render_targets.Add(TextureObject{ texture, sampler ? sampler : default_sampler });
			}

			new_resource.resource_id = textures.Add(TextureObject{ texture, sampler ? sampler : default_sampler });

			return graph_resources.size() - 1ull;
		}

		uint32_t Renderer::AddBufferResource(const gfx::BufferCreateInfo& create_info)
		{
			gfx::Buffer* buffer = gfx_context->CreateBuffer(create_info);
			auto handle = buffers.Add(BufferObject{ buffer });
			if (create_info.usage != gfx::BufferUsageFlag::eStagingBuffer && create_info.usage != gfx::BufferUsageFlag::eUniformBuffer)
			{
				auto& new_resource = graph_resources.emplace_back();
				new_resource.resource_type = RenderResourceTypeFlag::eBuffer;
				new_resource.resource_id = handle;

				return graph_resources.size() - 1ull;
			}

			return ~0u;
		}

		uint32_t Renderer::AddShaderPass(const ComputeShaderPassCreateInfo& create_info)
		{
			uint32_t pass_index = shader_passes.size();

			// Add per pass resource access container
			per_pass_resouce_access.emplace_back();
			per_pass_resource_bindings.emplace_back();

			// Collect render resource usage info
			CollectRencerResouces(pass_index, create_info);

			ComputePassCreateInfo pass_ci{};
			pass_ci.shader = resource_context->LoadShader(create_info.shader_name);

			auto& new_pass = shader_passes.emplace_back(std::make_unique<ComputePass>(*gfx_context, pass_ci));
			new_pass->SetExecutionContext(render_context.get());

			return pass_index;
		}

		uint32_t Renderer::AddShaderPass(const RenderShaderPassCreateInfo& create_info)
		{
			uint32_t pass_index = shader_passes.size();

			// Add per pass resource access container
			per_pass_resouce_access.emplace_back();
			per_pass_resource_bindings.emplace_back();

			// Collect render resource usage info
			CollectRencerResouces(pass_index, create_info);

			RenderPassCreateInfo pass_ci{};
			pass_ci.shader = resource_context->LoadShader(create_info.shader_name);
			pass_ci.state.primitive_topology = gfx::PrimitiveTopology::eTriangleList;
			//pass_ci.state.rasterization_state.front_face = gfx::FrontFace::eCW;

			// Collect color attachment formats
			auto& color_attachments = create_info.resource_attachments[utl::enum_cast(RenderGraphResourceAccess::eWrite)];
			for (auto& attachment : color_attachments)
			{
				auto& render_resource = graph_resources[attachment.render_resource_id];

				// Case for swapchain target
				if (render_resource.resource_id == ~0u)
				{
					auto& render_target = render_context->GetBackbuffer();
					pass_ci.state.color_formats[pass_ci.state.color_format_count++] = render_target.GetFormat();
				}
				else
				{
					auto& render_target = render_targets.Get(render_resource.resource_id);
					pass_ci.state.color_formats[pass_ci.state.color_format_count++] = render_target.texture->GetFormat();
				}

				// Init default color attachment setting
				pass_ci.state.color_blending_state.attachment_count++;
			}

			// Collect depth format
			if (create_info.depth_attachment.render_resource_id != ~0u)
			{
				per_pass_resouce_access[pass_index][utl::enum_cast(RenderGraphResourceAccess::eWrite)].emplace_back(create_info.depth_attachment.render_resource_id);
				per_pass_resource_bindings[pass_index].depth_attachment = create_info.depth_attachment;

				auto& render_resource = graph_resources[create_info.depth_attachment.render_resource_id];
				auto& depth_target = render_targets.Get(render_resource.resource_id);
				pass_ci.state.depth_format = depth_target.texture->GetFormat();
			}

			// Collect stencil format
			if (create_info.stencil_attachment.render_resource_id != ~0u)
			{
				per_pass_resouce_access[pass_index][utl::enum_cast(RenderGraphResourceAccess::eWrite)].emplace_back(create_info.stencil_attachment.render_resource_id);
				per_pass_resource_bindings[pass_index].stencil_attachment = create_info.stencil_attachment;

				auto& render_resource = graph_resources[create_info.stencil_attachment.render_resource_id];
				auto& stencil_target = render_targets.Get(render_resource.resource_id);
				pass_ci.state.stencil_format = stencil_target.texture->GetFormat();
			}

			auto& new_pass = shader_passes.emplace_back(std::make_unique<RenderPass>(*gfx_context, pass_ci));
			new_pass->SetExecutionContext(render_context.get());

			return pass_index;
		}

		void Renderer::Render(float delta_time)
		{
			BeginFrame();

			// Execute render graph
			EvaluateRenderGraph(delta_time);

			gfx::TextureBarrier texture_barrier{};
			gfx::ResourceBarrierInfo resource_barriers{};
			resource_barriers.texture_barriers = &texture_barrier;
			resource_barriers.texture_barrier_count = 1;

			texture_barrier.texture = &render_context->GetBackbuffer();
			texture_barrier.src_access = { gfx::AccessTypeFlag::eWrite, gfx::TextureLayout::eColorAttachmentOptimal };
			texture_barrier.dst_access = { gfx::AccessTypeFlag::eRead, gfx::TextureLayout::ePresent };

			auto& command_list = render_context->GetCurrentCommandList();
			command_list.ResourceBarrier(resource_barriers);

			EndFrame();
		}

		gfx::Context& Renderer::GetGFXContext()
		{
			return *gfx_context;
		}

		ExecutionContext& Renderer::GetRenderContext()
		{
			return *render_context;
		}

		ExecutionContext const& Renderer::GetRenderContext() const
		{
			return *render_context;
		}

		gfx::ResourceContext& Renderer::GetResourceContext()
		{
			return *resource_context;
		}

		gfx::ResourceContext const& Renderer::GetResourceContext() const
		{
			return *resource_context;
		}

		gfx::Sampler* Renderer::GetOrCreateSampler(const gfx::SamplerCreateInfo& create_info)
		{
			uint32_t sampler_hash = gfx::Hash(create_info);
			if (auto found = sampler_cache.find(sampler_hash); found != sampler_cache.end())
				return found->second;

			auto* new_sampler = sampler_cache[sampler_hash];
			new_sampler = gfx_context->CreateSampler(create_info);
			return new_sampler;
		}

		GeometryBuffer& Renderer::RequestInterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info)
		{
			uint32_t hash = Hash(create_info);

			if (auto found = interleaved_geometry_buffers.find(hash); found != interleaved_geometry_buffers.end())
			{
				return *found->second;
			}

			interleaved_geometry_buffers[hash] = std::make_unique<InterleavedGeometryBuffer>(*resource_context, create_info);
			return *interleaved_geometry_buffers[hash];
		}

		GeometryBuffer& Renderer::RequestDenterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info)
		{
			uint32_t hash = Hash(create_info);

			if (auto found = deinterleaved_geometry_buffers.find(hash); found != deinterleaved_geometry_buffers.end())
			{
				return *found->second;
			}

			deinterleaved_geometry_buffers[hash] = std::make_unique<DeinterleavedGeometryBuffer>(*resource_context, create_info);
			return *deinterleaved_geometry_buffers[hash];
		}


		RenderGraphNodeHeader* Renderer::AllocateStaticConditionNode()
		{
			return AllocateRenderGraphNode<RenderGraphStaticConditionNode>(RenderGraphNodeType::eStaticCondition);
		}

		RenderGraphNodeHeader* Renderer::AllocateDynamicConditionNode()
		{
			return AllocateRenderGraphNode<RenderGraphDynamicConditionNode>(RenderGraphNodeType::eDynamicCondition);
		}

		RenderGraphNodeHeader* Renderer::AllocateGroupNode()
		{
			return AllocateRenderGraphNode<RenderGraphGroupNode>(RenderGraphNodeType::eGroupNode);
		}

		RenderGraphNodeHeader* Renderer::AllocatePassNode()
		{
			return AllocateRenderGraphNode<RenderGraphPassNode>(RenderGraphNodeType::ePassNode);
		}

		gfx::TextureAccess Renderer::GetFinalTextureLayout(const RenderGraphResource& graph_resource)
		{
			if (graph_resource.resource_type & RenderResourceTypeFlag::eBackbuffer)
			{
				return { gfx::AccessTypeFlag::eRead, gfx::TextureLayout::ePresent };
			}
			else if (graph_resource.resource_type & RenderResourceTypeFlag::eAllImage)
			{
				gfx::Texture* texture = GetTextureResource(graph_resource);
				if (IsDepthStencilFormat(texture->GetFormat()))
				{
					return { gfx::AccessTypeFlag::eRead, gfx::TextureLayout::eDepthStencilReadOnlyOptimal };
				}

				return { gfx::AccessTypeFlag::eRead, gfx::TextureLayout::eShader };
			}
			else
			{
				assert(false && "RESOURCE TYPE IS NOT IMAGE");
			}

			return { {}, gfx::TextureLayout::eUndefined };
		}

		gfx::TextureAccess Renderer::GetIntermediateTextureLayout(const RenderGraphResource& graph_resource)
		{
			if (graph_resource.resource_type & RenderResourceTypeFlag::eStaticImage)
			{
				return { gfx::AccessTypeFlag::eWrite, gfx::TextureLayout::eShader };
			}
			else if(graph_resource.resource_type & RenderResourceTypeFlag::eAllImage)
			{
				gfx::Texture* texture = GetTextureResource(graph_resource);
				if (IsDepthStencilFormat(texture->GetFormat()))
				{
					return { gfx::AccessTypeFlag::eWrite, gfx::TextureLayout::eDepthStencilAttachmentOptimal };
				}

				return { gfx::AccessTypeFlag::eWrite, gfx::TextureLayout::eColorAttachmentOptimal };
			}
			else
			{
				assert(false && "RESOURCE TYPE IS NOT IMAGE");
			}

			return { {}, gfx::TextureLayout::eUndefined };
		}

		gfx::TextureAccess Renderer::GetCurrentTextureLayout(const RenderGraphResource& graph_resource)
		{
			if (graph_resource.resource_type & RenderResourceTypeFlag::eBackbuffer)
			{
				return backbuffer_layout;
			}
			else if (graph_resource.resource_type & RenderResourceTypeFlag::eAllImage)
			{
				auto& resource = render_targets.Get(graph_resource.resource_id);
				return resource.current_access;
			}
			else
			{
				assert(false && "RESOURCE TYPE IS NOT IMAGE");
			}

			return { {}, gfx::TextureLayout::eUndefined };
		}

		void Renderer::UpdateTextureLayout(const RenderGraphResource& graph_resource, gfx::TextureAccess new_layout)
		{
			if (graph_resource.resource_type & RenderResourceTypeFlag::eBackbuffer)
			{
				backbuffer_layout = new_layout;
			}
			else if (graph_resource.resource_type & RenderResourceTypeFlag::eAllImage)
			{
				auto& resource = render_targets.Get(graph_resource.resource_id);
				resource.current_access = new_layout;
			}
			else
			{
				assert(false && "RESOURCE TYPE IS NOT IMAGE");
			}
		}

		gfx::Texture* Renderer::GetTextureResource(const RenderGraphResource& graph_resource)
		{
			if (graph_resource.resource_type & RenderResourceTypeFlag::eBackbuffer)
			{
				return &render_context->GetBackbuffer();
			}
			else if (graph_resource.resource_type & RenderResourceTypeFlag::eAllImage)
			{
				auto& resource = render_targets.Get(graph_resource.resource_id);
				return resource.texture;
			}
			else
			{
				assert(false && "RESOURCE TYPE IS NOT IMAGE");
			}

			return nullptr;
		}

		void Renderer::CollectRencerResouces(uint32_t pass_index, const ShaderPassCreateInfo& pass_ci)
		{
			// i here is resource access type
			for (int32_t i = 0; i < 3ull; ++i)
			{
				auto& per_pass_access = per_pass_resouce_access[pass_index][i];
				auto& per_pass_bindings = per_pass_resource_bindings[pass_index];

				// Process bindings
				for (auto& resource_binding : pass_ci.resource_bindings[i])
				{
					// Get graph resource and update that this one was used in concrete pass
					auto& graph_resource = graph_resources[resource_binding.render_resource_id];
					graph_resource.usages[i].emplace(pass_index);

					per_pass_access.emplace_back(resource_binding.render_resource_id);
					per_pass_bindings.bindings[i].emplace_back(resource_binding);
				}

				// Process attachments
				for (auto& resource_attachment : pass_ci.resource_attachments[i])
				{
					auto& graph_resource = graph_resources[resource_attachment.render_resource_id];
					graph_resource.usages[i].emplace(pass_index);

					per_pass_access.emplace_back(resource_attachment.render_resource_id);
					per_pass_bindings.attachments[i].emplace_back(resource_attachment);
				}
			}
		}

		void Renderer::DestroyRenderGraph()
		{
			for (auto& node : render_graph)
			{
				DestoyRenderGraphNode(node);
			}
		}

		void Renderer::DestoyRenderGraphNode(RenderGraphNodeHeader* node)
		{
			if (node)
			{
				switch (node->type)
				{
				case RenderGraphNodeType::eStaticCondition:
				{
					auto* static_condition_node = CastRenderGraphNode<RenderGraphStaticConditionNode>(node);
					DestoyRenderGraphNode(static_condition_node->node);
					DeallocateRenderGraphNode(&static_condition_node->node);
				} break;
				case RenderGraphNodeType::eDynamicCondition:
				{
					auto* dynamic_condition_node = CastRenderGraphNode<RenderGraphDynamicConditionNode>(node);

					DestoyRenderGraphNode(dynamic_condition_node->pass);
					DeallocateRenderGraphNode(&dynamic_condition_node->pass);

					DestoyRenderGraphNode(dynamic_condition_node->fail);
					DeallocateRenderGraphNode(&dynamic_condition_node->fail);

				} break;
				case RenderGraphNodeType::eGroupNode:
				{
					auto* group_node = CastRenderGraphNode<RenderGraphGroupNode>(node);

					for (auto& node : group_node->nodes)
					{
						DestoyRenderGraphNode(node);
						DeallocateRenderGraphNode(&node);
					}

				} break;
				case RenderGraphNodeType::ePassNode:
				{
					auto* pass_node = CastRenderGraphNode<RenderGraphPassNode>(node);

					// Deallocate shader pass
					auto& pass = shader_passes[pass_node->pass_index];
					pass.reset();
				} break;
				}
			}
		}

		void Renderer::EvaluateShaderPass(uint32_t pass_index)
		{
			std::array<gfx::BufferBarrier, 64ull> buffer_barriers{};
			std::array<gfx::TextureBarrier, 64ull> texture_barriers{};
			std::array<gfx::MemoryBarrier, 64ull> memory_barriers{};

			gfx::ResourceBarrierInfo resource_barriers{};
			resource_barriers.buffer_barriers = buffer_barriers.data();
			resource_barriers.texture_barriers = texture_barriers.data();
			resource_barriers.memory_barriers = memory_barriers.data();

			auto& shader_pass = shader_passes[pass_index];
			auto& pass_bindings = per_pass_resource_bindings[pass_index];

			for (int32_t access_idx = 0; access_idx < 3ull; ++access_idx)
			{
				RenderGraphResourceAccess resource_access{ static_cast<RenderGraphResourceAccess>(access_idx) };
				const auto& resource_bindings = per_pass_resouce_access[pass_index][access_idx];

				// Start from resource bindings
				for (const auto& resource_binding : pass_bindings.bindings[access_idx])
				{
					auto& render_resource = graph_resources[resource_binding.render_resource_id];

					// Bind resource
					ShaderResourceBinding shader_resource_binding{};
					shader_resource_binding.binding_point = resource_binding.bind_point;
					shader_resource_binding.subresource = resource_binding.subresource_index;

					// Do not need to sync static images
					if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage)
					{
						// Bind texture and continue
						auto& resource = textures.Get(render_resource.resource_id);
						shader_resource_binding.texture = resource.texture;
						shader_resource_binding.sampler = resource.sampler;
						shader_pass->BindResource(shader_resource_binding);

						continue;
					}

					// Ignore read only resource
					if (IsXOnlyAccess(render_resource, RenderGraphResourceAccess::eRead))
						continue;

					// Graph also should support work with subresources
					uint32_t subresource_key = resource_binding.subresource_index == ~0u ? 0u : resource_binding.subresource_index;
					auto resource_key = std::make_pair(resource_binding.render_resource_id, subresource_key);

					auto& last_resource_access = resource_access_map[resource_key];

					gfx::AccessTypeFlag access_flag = GetAccessFlagType(resource_access);

					// Ignoring read after read barriers
					if ((last_resource_access.access & access_flag) && access_flag == gfx::AccessTypeFlag::eRead)
						continue;

					bool is_write_after_write = ((last_resource_access.access & access_flag) && access_flag == gfx::AccessTypeFlag::eWrite);
					bool is_change_pipeline_ownership = last_resource_access.pass_type == utl::enum_cast(shader_pass->GetPassType()) && is_write_after_write;

					if (render_resource.resource_type & RenderResourceTypeFlag::eBuffer)
					{
						auto& resource = buffers.Get(render_resource.resource_id);
						auto& barrier = buffer_barriers[resource_barriers.buffer_barrier_count++];
						barrier.buffer = resource.buffer;
						barrier.src_state = resource.resource_state;

						const gfx::AccessFlags read_flags = gfx::AccessFlag::eShaderRead | gfx::AccessFlag::eShaderStorageRead | gfx::AccessFlag::eShaderSampledRead;
						const gfx::AccessFlags write_flags = gfx::AccessFlag::eShaderWrite | gfx::AccessFlag::eShaderStorageWrite;

						if (access_flag == gfx::AccessTypeFlag::eRead)
							barrier.dst_state.access = read_flags | (shader_pass->GetPassType() == ShaderPassType::eCompute ? gfx::AccessFlag::eIndirectCommandRead : gfx::AccessFlag::eNone);
						else
							barrier.dst_state.access = write_flags;

						barrier.dst_state.stage = shader_pass->GetPassType() == ShaderPassType::eCompute ? gfx::StageFlag::eComputeShader : gfx::StageFlag::eAllGraphics;

						resource.resource_state = barrier.dst_state;
					}
					else
					{
						auto& barrier = texture_barriers[resource_barriers.texture_barrier_count++];
						barrier.src_access = GetCurrentTextureLayout(render_resource);
						barrier.texture = GetTextureResource(render_resource);

						auto final_layout = GetFinalTextureLayout(render_resource);
						auto intermediate_layout = GetIntermediateTextureLayout(render_resource);

						if (last_resource_access.access == 0)
							barrier.dst_access = access_flag == gfx::AccessTypeFlag::eRead ? final_layout : intermediate_layout;
						else if (access_flag != gfx::AccessTypeFlag::eWrite && IsROAccessOnlyRemain(render_resource, pass_index))
							barrier.dst_access = final_layout;
						else if (is_change_pipeline_ownership)
							barrier.dst_access = intermediate_layout;
					}

					// Try bind render resource
					if (render_resource.resource_type & RenderResourceTypeFlag::eAllImage)
					{
						auto& resource = render_targets.Get(render_resource.resource_id);
						shader_resource_binding.texture = resource.texture;
						shader_resource_binding.sampler = resource.sampler;
					}
					else if (render_resource.resource_type & RenderResourceTypeFlag::eBuffer)
					{
						auto& resource = buffers.Get(render_resource.resource_id);
						shader_resource_binding.buffer = resource.buffer;
					}
					else
					{
						assert(false && "UNKNOWN RESOURCE TYPE");
					}

					shader_pass->BindResource(shader_resource_binding);
				}

				// For attachments (write only)
				if (!pass_bindings.attachments[access_idx].empty())
				{
					auto* render_pass = static_cast<RenderPass*>(shader_pass.get());
					for (const auto& resource_attachment : pass_bindings.attachments[access_idx])
					{
						auto& render_resource = graph_resources[resource_attachment.render_resource_id];

						// Wtf? Trying write to static resource????
						if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage)
						{
							log_error("Trying bind static texture as color attachment!");
							assert(false && "INCORRECT OUTPUT RESOURCE BINDING!");
							continue;
						}

						// Graph also should support work with subresources
						auto resource_key = std::make_pair(resource_attachment.render_resource_id, 0u);

						auto& last_resource_access = resource_access_map[resource_key];

						gfx::AccessTypeFlag access_flag = GetAccessFlagType(resource_access);

						bool is_backbuffer_resource = resource_attachment.render_resource_id == backbuffer_resource;
						bool is_write_after_write = ((last_resource_access.access & access_flag) && access_flag == gfx::AccessTypeFlag::eWrite);
						bool is_change_pipeline_ownership = last_resource_access.pass_type == utl::enum_cast(shader_pass->GetPassType()) && is_write_after_write;

						gfx::Attachment attachment{};
						attachment.load_op = is_write_after_write ? gfx::LoadOp::eLoad : gfx::LoadOp::eClear;
						attachment.store_op = !render_resource.usages[utl::enum_cast(RenderGraphResourceAccess::eRead)].empty() || is_backbuffer_resource ? gfx::StoreOp::eStore : gfx::StoreOp::eDontCare;
						attachment.clear_color = resource_attachment.clear_color;

						auto& barrier = texture_barriers[resource_barriers.texture_barrier_count++];
						barrier.src_access = GetCurrentTextureLayout(render_resource);
						barrier.texture = GetTextureResource(render_resource);

						auto final_layout = GetFinalTextureLayout(render_resource);
						auto intermediate_layout = GetIntermediateTextureLayout(render_resource);

						if (last_resource_access.access == 0)
							barrier.dst_access = access_flag == gfx::AccessTypeFlag::eRead ? final_layout : intermediate_layout;
						else if (access_flag != gfx::AccessTypeFlag::eWrite && IsROAccessOnlyRemain(render_resource, pass_index))
							barrier.dst_access = is_backbuffer_resource ? intermediate_layout : final_layout;
						else if (is_change_pipeline_ownership)
							barrier.dst_access = intermediate_layout;

						UpdateTextureLayout(render_resource, barrier.dst_access);

						attachment.texture = barrier.texture;

						render_pass->AddColorAttachment(attachment);
					}
				}
			}

			if (pass_bindings.depth_attachment.render_resource_id != ~0u)
			{
				auto& resource_attachment = pass_bindings.depth_attachment;
				auto& render_resource = graph_resources[resource_attachment.render_resource_id];

				// Graph also should support work with subresources
				auto resource_key = std::make_pair(resource_attachment.render_resource_id, 0u);

				auto& last_resource_access = resource_access_map[resource_key];

				gfx::AccessTypeFlag access_flag = gfx::AccessTypeFlag::eWrite;

				bool is_write_after_write = ((last_resource_access.access & access_flag) && access_flag == gfx::AccessTypeFlag::eWrite);
				bool is_change_pipeline_ownership = last_resource_access.pass_type == utl::enum_cast(shader_pass->GetPassType()) && is_write_after_write;

				gfx::Attachment attachment{};
				attachment.load_op = is_write_after_write ? gfx::LoadOp::eLoad : gfx::LoadOp::eClear;
				attachment.store_op = !render_resource.usages[utl::enum_cast(RenderGraphResourceAccess::eRead)].empty() ? gfx::StoreOp::eStore : gfx::StoreOp::eDontCare;
				attachment.clear_color = resource_attachment.clear_color;

				auto& barrier = texture_barriers[resource_barriers.texture_barrier_count++];
				barrier.texture = GetTextureResource(render_resource);
				barrier.src_access = GetCurrentTextureLayout(render_resource);

				auto final_layout = GetFinalTextureLayout(render_resource);
				auto intermediate_layout = GetIntermediateTextureLayout(render_resource);

				if (last_resource_access.access == 0)
					barrier.dst_access = access_flag == gfx::AccessTypeFlag::eRead ? final_layout : intermediate_layout;
				else if (access_flag != gfx::AccessTypeFlag::eWrite && IsROAccessOnlyRemain(render_resource, pass_index))
					barrier.dst_access = final_layout;
				else if (is_change_pipeline_ownership)
					barrier.dst_access = intermediate_layout;

				UpdateTextureLayout(render_resource, barrier.dst_access);

				attachment.texture = barrier.texture;

				auto* render_pass = static_cast<RenderPass*>(shader_pass.get());
				render_pass->SetDepthAttachment(attachment);
			}

			if (pass_bindings.stencil_attachment.render_resource_id != ~0u)
			{
				auto& resource_attachment = pass_bindings.stencil_attachment;
				auto& render_resource = graph_resources[resource_attachment.render_resource_id];

				// Graph also should support work with subresources
				auto resource_key = std::make_pair(resource_attachment.render_resource_id, 0u);

				auto& last_resource_access = resource_access_map[resource_key];

				gfx::AccessTypeFlag access_flag = gfx::AccessTypeFlag::eWrite;

				bool is_write_after_write = ((last_resource_access.access & access_flag) && access_flag == gfx::AccessTypeFlag::eWrite);
				bool is_change_pipeline_ownership = last_resource_access.pass_type == utl::enum_cast(shader_pass->GetPassType()) && is_write_after_write;

				gfx::Attachment attachment{};
				attachment.load_op = is_write_after_write ? gfx::LoadOp::eLoad : gfx::LoadOp::eClear;
				attachment.store_op = !render_resource.usages[utl::enum_cast(RenderGraphResourceAccess::eRead)].empty() ? gfx::StoreOp::eStore : gfx::StoreOp::eDontCare;
				attachment.clear_color = resource_attachment.clear_color;

				auto& barrier = texture_barriers[resource_barriers.texture_barrier_count++];
				barrier.texture = GetTextureResource(render_resource);
				barrier.src_access = GetCurrentTextureLayout(render_resource);

				auto final_layout = GetFinalTextureLayout(render_resource);
				auto intermediate_layout = GetIntermediateTextureLayout(render_resource);

				if (last_resource_access.access == 0)
					barrier.dst_access = access_flag == gfx::AccessTypeFlag::eRead ? final_layout : intermediate_layout;
				else if (access_flag != gfx::AccessTypeFlag::eWrite && IsROAccessOnlyRemain(render_resource, pass_index))
					barrier.dst_access = final_layout;
				else if (is_change_pipeline_ownership)
					barrier.dst_access = intermediate_layout;

				UpdateTextureLayout(render_resource, barrier.dst_access);

				attachment.texture = barrier.texture;

				auto* render_pass = static_cast<RenderPass*>(shader_pass.get());
				render_pass->SetStencilAttachment(attachment);
			}

			auto& command_list = render_context->GetCurrentCommandList();
			command_list.ResourceBarrier(resource_barriers);
		}

		void Renderer::EvaluateRenderGraph(float delta_time)
		{
			resource_access_map.clear();

			for (auto& node : render_graph)
			{
				EvaluateRenderGraphNode(node, delta_time);
			}
		}

		void Renderer::EvaluateRenderGraphNode(RenderGraphNodeHeader* node, float delta_time)
		{
			if (node)
			{
				switch (node->type)
				{
				case RenderGraphNodeType::eStaticCondition:
				{
					auto* static_condition_node = CastRenderGraphNode<RenderGraphStaticConditionNode>(node);
					EvaluateRenderGraphNode(static_condition_node->node, delta_time);
				} break;
				case RenderGraphNodeType::eDynamicCondition:
				{
					auto* dynamic_condition_node = CastRenderGraphNode<RenderGraphDynamicConditionNode>(node);
					if (dynamic_condition_node->condition_fn())
						EvaluateRenderGraphNode(dynamic_condition_node->pass, delta_time);
					else
						EvaluateRenderGraphNode(dynamic_condition_node->fail, delta_time);

				} break;
				case RenderGraphNodeType::eGroupNode:
				{
					auto* group_node = CastRenderGraphNode<RenderGraphGroupNode>(node);

					// TODO: Add execution context selection. Now it always render context
					auto& command_list = render_context->GetCurrentCommandList();
					command_list.BeginMarker(group_node->group_name.c_str(), group_node->color);

					for (auto& node : group_node->nodes)
						EvaluateRenderGraphNode(node, delta_time);

					command_list.EndMarker();
				} break;
				case RenderGraphNodeType::ePassNode:
				{
					auto* pass_node = CastRenderGraphNode<RenderGraphPassNode>(node);

					auto& pass = shader_passes[pass_node->pass_index];
					// TODO: Add can execute condition
					//if (!pass->CanExecute())
					//	return;

					// Temporary code
					if (pass->GetPassType() == ShaderPassType::eGraphics)
					{
						auto& swapchain = render_context->GetSwapchain();

						uint32_t width, height;
						swapchain.GetSurfaceExtent(width, height);

						auto* graphics_pass = static_cast<RenderPass*>(pass.get());
						graphics_pass->SetRenderResolution(width, height);
					}

					EvaluateShaderPass(pass_node->pass_index);
					pass->Execute(delta_time);
				} break;
				}
			}
		}


		void Renderer::BuildGraph()
		{
			// Register backbuffer resource

			backbuffer_resource = graph_resources.size();

			auto& backbuffer_resource_binding = graph_resources.emplace_back();
			backbuffer_resource_binding.resource_type = RenderResourceTypeFlag::eBackbuffer;

			// Create shader passes
			RenderShaderPassCreateInfo create_info{};
			create_info.shader_name = "testing:draw_uv";
			create_info.AddWriteResource(backbuffer_resource, { 1.f, 0.5f, 1.f, 1.f });

			auto* new_node = AllocatePassNode();
			auto* pass_node = CastRenderGraphNode<RenderGraphPassNode>(new_node);
			pass_node->pass_index = AddShaderPass(create_info);

			render_graph.push_back(new_node);
		}

		void Renderer::BeginFrame()
		{
			bool surface_updated{ false };
			auto& command_buffer = render_context->Begin(surface_updated);
			if (surface_updated)
			{
				UpdateRenderTargets();
			}

			uint64_t timestamps[2] = {};
			timestamp_query->GetData(0u, &timestamps[0]);

			uint64_t elapsed_time = timestamps[1] - timestamps[0];
			if (timestamps[1] <= timestamps[0])
			{
				elapsed_time = 0ull;
			}

			gpu_delta_time = static_cast<float>(elapsed_time) * timestamp_frequency / 1000000000.f;

			command_buffer.ResetQuery(timestamp_query, 0u, 2u);
			command_buffer.BeginQuery(timestamp_query, 0u);
		}

		void Renderer::EndFrame()
		{
			gfx::SemaphoreSubmitInfo upload_wait_semaphore{};
			//auto upload_avaliable = render_context->GetUploadAvaliableSemaphore();
			//upload_wait_semaphore = upload_context->Submit(&upload_avaliable, upload_avaliable.semaphore ? 1 : 0);
			upload_wait_semaphore = resource_context->Submit(nullptr, 0);

			auto& command_list = render_context->GetCurrentCommandList();
			command_list.EndQuery(timestamp_query, 1u);

			render_context->EndFrame(upload_wait_semaphore);

			// Reset local render targets states
			for (auto& target : render_targets.data())
			{
				target.current_access = { {}, gfx::TextureLayout::eUndefined };
				target.texture->Reset();
			}

			backbuffer_layout = { {}, gfx::TextureLayout::eUndefined };

			for (auto& buffer : buffers.data())
			{
				buffer.resource_state = { gfx::AccessFlag::eNone, gfx::StageFlag::eAllCommands };
			}
		}

		void Renderer::UpdateRenderTargets()
		{
			// Update/resize render targets here if it's needed
			for (auto& target : render_targets.data())
			{

			}
		}
	}
}