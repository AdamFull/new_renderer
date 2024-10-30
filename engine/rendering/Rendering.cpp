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

			// TODO: now it creates just default one
			gfx::SharedDescriptorCreateInfo shared_descriptor_ci{};
			shared_descriptor_ci.descriptions.emplace_back(gfx::DescriptorType::eUniformBuffer, 0u, 1u);
			shared_descriptor_ci.descriptions.emplace_back(gfx::DescriptorType::eUniformBuffer, 1u, 1u);
			shared_descriptor_ci.descriptions.emplace_back(gfx::DescriptorType::eUniformBuffer, 2u, 1u);
			shared_descriptor_ci.descriptions.emplace_back(gfx::DescriptorType::eCombinedImageSampler, 3u, 16384u);

			shared_descriptor = gfx_context->CreateSharedDescriptor(shared_descriptor_ci);

			// Temporary func to create test render graph
			BuildGraph();
		}

		Renderer::~Renderer()
		{
			// Destroy render graph including shader passes
			DestroyRenderGraph();

			// Free textures
			for (auto& texture : textures.data())
			{
				gfx_context->Destroy(texture.texture);
			}

			// Free render targets
			for (auto& texture : render_targets.data())
			{
				gfx_context->Destroy(texture.texture);
			}

			// Free buffers
			for (auto& buffer : buffers.data())
			{
				gfx_context->Destroy(buffer.buffer);
			}

			// Free samplers
			for (auto& [hash, sampler] : sampler_cache)
			{
				gfx_context->Destroy(sampler);
			}

			gfx_context->Destroy(shared_descriptor);
			gfx_context->Destroy(default_sampler);
			gfx_context->Destroy(timestamp_query);
		}

		uint32_t Renderer::AddRenderResource(const std::string& path, gfx::Sampler* sampler)
		{
			auto& new_resource = graph_resources.emplace_back();
			new_resource.resource_type = RenderResourceTypeFlag::eStaticImage;

			gfx::Texture* texture = resource_context->LoadTexture(path);
			new_resource.resource_id = textures.Add(TextureObject{ texture, sampler ? sampler : default_sampler });

			new_resource.current_state = gfx::ResourceStateFlag::eShaderRead;

			return graph_resources.size() - 1ull;
		}

		uint32_t Renderer::AddRenderResource(const gfx::TextureCreateInfo& create_info, gfx::Sampler* sampler)
		{
			auto& new_resource = graph_resources.emplace_back();

			// TODO: What if it's storage target and render target at the same time?
			if (create_info.usage & gfx::TextureUsageFlag::eStorage)
			{
				new_resource.intermediate_state = gfx::ResourceStateFlag::eShaderWrite;
				new_resource.final_state = gfx::ResourceStateFlag::eShaderRead;
				new_resource.resource_type = RenderResourceTypeFlag::eStorageTarget;
			}
			else if (create_info.usage & gfx::TextureUsageFlag::eRenderTarget)
			{
				new_resource.intermediate_state = IsDepthStencilFormat(create_info.format) ? gfx::ResourceStateFlag::eDepthStencilAttachmentWrite : gfx::ResourceStateFlag::eColorAttachmentWrite;
				new_resource.final_state = IsDepthStencilFormat(create_info.format) ?  gfx::ResourceStateFlag::eDepthStencilAttachmentRead : gfx::ResourceStateFlag::eShaderRead;
				new_resource.resource_type = RenderResourceTypeFlag::eRenderTarget;
			}
			else
				new_resource.resource_type = RenderResourceTypeFlag::eStaticImage;

			gfx::Texture* texture = gfx_context->CreateTexture(create_info);
			if ((create_info.usage & gfx::TextureUsageFlag::eRenderTarget) || (create_info.usage & gfx::TextureUsageFlag::eStorage))
			{
				new_resource.resource_id = render_targets.Add(TextureObject{ texture, sampler ? sampler : default_sampler, create_info.usage, create_info.resizable });
			}
			else
			{
				new_resource.resource_id = textures.Add(TextureObject{ texture, sampler ? sampler : default_sampler, create_info.usage, create_info.resizable });
			}

			new_resource.subresource_states.fill(gfx::ResourceStateFlag::eUndefined);

			return graph_resources.size() - 1ull;
		}

		uint32_t Renderer::AddRenderResource(const gfx::BufferCreateInfo& create_info)
		{
			gfx::Buffer* buffer = gfx_context->CreateBuffer(create_info);
			auto handle = buffers.Add(BufferObject{ buffer });
			if (create_info.usage != gfx::BufferUsageFlag::eStagingBuffer && create_info.usage != gfx::BufferUsageFlag::eUniformBuffer)
			{
				auto& new_resource = graph_resources.emplace_back();
				new_resource.resource_type = RenderResourceTypeFlag::eBuffer;
				new_resource.resource_id = handle;

				new_resource.current_state = gfx::ResourceStateFlag::eUndefined;
				new_resource.intermediate_state = gfx::ResourceStateFlag::eShaderWrite;
				new_resource.final_state = gfx::ResourceStateFlag::eShaderRead;

				return graph_resources.size() - 1ull;
			}

			return ~0u;
		}

		uint32_t Renderer::AddShaderPass(const ComputeShaderPassCreateInfo& create_info)
		{
			uint32_t pass_index = shader_passes.size();

			// Add per pass resource access container
			per_pass_resource_bindings.emplace_back();

			// Collect render resource usage info
			CollectRencerResouces(pass_index, create_info);

			ComputePassCreateInfo pass_ci{};
			pass_ci.debug_name = create_info.shader_name;
			pass_ci.shader = resource_context->LoadShader(create_info.shader_name);
			pass_ci.one_time = create_info.one_time;
			pass_ci.shared_descriptor = shared_descriptor;
			pass_ci.draw_callback = std::move(create_info.draw_callback);

			auto& new_pass = shader_passes.emplace_back(std::make_unique<ComputePass>(*gfx_context, pass_ci));
			new_pass->SetExecutionContext(render_context.get());

			return pass_index;
		}

		uint32_t Renderer::AddShaderPass(const RenderShaderPassCreateInfo& create_info)
		{
			uint32_t pass_index = shader_passes.size();

			// Add per pass resource access container
			per_pass_resource_bindings.emplace_back();

			// Collect render resource usage info
			CollectRencerResouces(pass_index, create_info);

			RenderPassCreateInfo pass_ci{};
			pass_ci.debug_name = create_info.shader_name;
			pass_ci.shader = resource_context->LoadShader(create_info.shader_name);
			pass_ci.shared_descriptor = shared_descriptor;
			pass_ci.state.primitive_topology = gfx::PrimitiveTopology::eTriangleList;
			pass_ci.draw_callback = std::move(create_info.draw_callback);
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

					// Init default color attachment setting
					pass_ci.state.color_blending_state.attachment_count++;
				}
				else
				{
					auto& render_target = render_targets.Get(render_resource.resource_id);
					auto rt_format = render_target.texture->GetFormat();
					if (attachment.flags & ResourceAttachmentTypeFlag::eDepth)
					{
						pass_ci.state.depth_format = rt_format;
					}
					else if (attachment.flags & ResourceAttachmentTypeFlag::eStencil)
					{
						pass_ci.state.stencil_format = rt_format;
					}
					else
					{
						pass_ci.state.color_formats[pass_ci.state.color_format_count++] = rt_format;

						// Init default color attachment setting
						pass_ci.state.color_blending_state.attachment_count++;
					}
				}
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
			// TODO: get current resource state from resource itself
			texture_barrier.src_state = gfx::ResourceStateFlag::eColorAttachmentWrite;
			texture_barrier.dst_state = gfx::ResourceStateFlag::ePresent;

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

			return sampler_cache[sampler_hash] = gfx_context->CreateSampler(create_info);
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

		GeometryBuffer& Renderer::RequestDeinterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info)
		{
			uint32_t hash = Hash(create_info);

			if (auto found = deinterleaved_geometry_buffers.find(hash); found != deinterleaved_geometry_buffers.end())
			{
				return *found->second;
			}

			deinterleaved_geometry_buffers[hash] = std::make_unique<DeinterleavedGeometryBuffer>(*resource_context, create_info);
			return *deinterleaved_geometry_buffers[hash];
		}


		RenderGraphNodeHeader* Renderer::CreateShaderPassNode(const ComputeShaderPassCreateInfo& create_info)
		{
			auto* node = AllocateRenderGraphNode<RenderGraphPassNode>(RenderGraphNodeType::ePassNode);
			auto* pass_node = CastRenderGraphNode<RenderGraphPassNode>(node);
			pass_node->pass_index = AddShaderPass(create_info);
			return node;
		}

		RenderGraphNodeHeader* Renderer::CreateShaderPassNode(const RenderShaderPassCreateInfo& create_info)
		{
			auto* node = AllocateRenderGraphNode<RenderGraphPassNode>(RenderGraphNodeType::ePassNode);
			auto* pass_node = CastRenderGraphNode<RenderGraphPassNode>(node);
			pass_node->pass_index = AddShaderPass(create_info);
			return node;
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
				auto& per_pass_bindings = per_pass_resource_bindings[pass_index];

				// Process bindings
				for (auto& resource_binding : pass_ci.resource_bindings[i])
				{
					// Get graph resource and update that this one was used in concrete pass
					auto& graph_resource = graph_resources[resource_binding.render_resource_id];
					graph_resource.usages[i].emplace(pass_index);

					per_pass_bindings.bindings[i].emplace_back(resource_binding);
				}

				// Process attachments
				for (auto& resource_attachment : pass_ci.resource_attachments[i])
				{
					auto& graph_resource = graph_resources[resource_attachment.render_resource_id];
					graph_resource.usages[i].emplace(pass_index);

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
			static gfx::AccessTypeFlag access_map[] = { gfx::AccessTypeFlag::eRead, gfx::AccessTypeFlag::eWrite, gfx::AccessTypeFlag::eRW };

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
				gfx::AccessTypeFlag access_flag = access_map[access_idx];

				// Start from resource bindings
				for (const auto& resource_binding : pass_bindings.bindings[access_idx])
				{
					auto& render_resource = graph_resources[resource_binding.render_resource_id];

					// Do not need to sync static images
					if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage)
					{
						continue;
					}

					// Ignore read only resource
					if (IsXOnlyAccess(render_resource, RenderGraphResourceAccess::eRead))
						continue;

					// Graph also should support work with subresources
					uint32_t subresource_key = resource_binding.subresource_index == ~0u ? 0u : resource_binding.subresource_index;
					auto resource_key = std::make_pair(resource_binding.render_resource_id, subresource_key);

					auto& last_resource_access = resource_access_map[resource_key];

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
						barrier.src_state = render_resource.GetState(resource_binding.subresource_index);

						if (access_flag == gfx::AccessTypeFlag::eRead)
							barrier.dst_state = render_resource.final_state;
						else
							barrier.dst_state = render_resource.intermediate_state;

							render_resource.SetState(barrier.dst_state, resource_binding.subresource_index);
					}
					else
					{
						auto& barrier = texture_barriers[resource_barriers.texture_barrier_count++];
						barrier.src_state = render_resource.GetState(resource_binding.subresource_index);
						barrier.texture = GetTextureResource(render_resource);
						barrier.subresource_index = resource_binding.subresource_index;

						if (last_resource_access.access == 0)
							barrier.dst_state = access_flag == gfx::AccessTypeFlag::eRead ? render_resource.final_state : render_resource.intermediate_state;
						else if (access_flag != gfx::AccessTypeFlag::eWrite && IsROAccessOnlyRemain(render_resource, pass_index))
							barrier.dst_state = render_resource.final_state;
						else if (is_change_pipeline_ownership)
							barrier.dst_state = render_resource.intermediate_state;

						render_resource.SetState(barrier.dst_state, resource_binding.subresource_index);
					}

					last_resource_access.access = access_flag;
					last_resource_access.pass_type = utl::enum_cast(shader_pass->GetPassType());
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

						bool is_backbuffer_resource = resource_attachment.render_resource_id == backbuffer_resource;
						bool is_write_after_write = ((last_resource_access.access & access_flag) && access_flag == gfx::AccessTypeFlag::eWrite);
						bool is_change_pipeline_ownership = last_resource_access.pass_type == utl::enum_cast(shader_pass->GetPassType()) && is_write_after_write;

						gfx::Attachment attachment{};
						attachment.load_op = is_write_after_write ? gfx::LoadOp::eLoad : gfx::LoadOp::eClear;
						attachment.store_op = !render_resource.usages[utl::enum_cast(RenderGraphResourceAccess::eRead)].empty() || is_backbuffer_resource ? gfx::StoreOp::eStore : gfx::StoreOp::eDontCare;
						attachment.clear_color = resource_attachment.clear_color;

						auto& barrier = texture_barriers[resource_barriers.texture_barrier_count++];
						barrier.src_state = render_resource.GetState();
						barrier.texture = GetTextureResource(render_resource);

						if (last_resource_access.access == 0)
							barrier.dst_state = access_flag == gfx::AccessTypeFlag::eRead ? render_resource.final_state : render_resource.intermediate_state;
						else if (access_flag != gfx::AccessTypeFlag::eWrite && IsROAccessOnlyRemain(render_resource, pass_index))
							barrier.dst_state = is_backbuffer_resource ? render_resource.intermediate_state : render_resource.final_state;
						else if (is_change_pipeline_ownership)
							barrier.dst_state = render_resource.intermediate_state;

						render_resource.SetState(barrier.dst_state);

						attachment.texture = barrier.texture;

						if (resource_attachment.flags & ResourceAttachmentTypeFlag::eDepthStencil)
							render_pass->SetDepthStencilAttachment(attachment);
						else if (resource_attachment.flags & ResourceAttachmentTypeFlag::eDepth)
							render_pass->SetDepthAttachment(attachment);
						else if (resource_attachment.flags & ResourceAttachmentTypeFlag::eStencil)
							render_pass->SetStencilAttachment(attachment);
						else
							render_pass->AddColorAttachment(attachment);

						last_resource_access.access = access_flag;
						last_resource_access.pass_type = utl::enum_cast(shader_pass->GetPassType());
					}
				}
			}

			// Translate textures
			auto& command_list = render_context->GetCurrentCommandList();
			command_list.ResourceBarrier(resource_barriers);

			// Bind pass resources
			for (int32_t access_idx = 0; access_idx < 3ull; ++access_idx)
			{
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
						auto& resource = textures.Get(render_resource.resource_id);
						shader_resource_binding.texture = resource.texture;
						shader_resource_binding.sampler = resource.sampler;
					}
					else if ((render_resource.resource_type & RenderResourceTypeFlag::eRenderTarget) || (render_resource.resource_type & RenderResourceTypeFlag::eStorageTarget))
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
						assert(false && "THIS RESOURCE TYPE IS NOT SUPPORTED FOR BINDING");
					}

					shader_pass->BindResource(shader_resource_binding);
				}
			}
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

					if (!pass->CanExecute())
						return;

					EvaluateShaderPass(pass_node->pass_index);
					pass->Execute(delta_time);
				} break;
				}
			}
		}


		void Renderer::BuildGraph()
		{
			using namespace utl;

			// Register backbuffer resource

			backbuffer_resource = graph_resources.size();

			auto& backbuffer_resource_binding = graph_resources.emplace_back();
			backbuffer_resource_binding.resource_type = RenderResourceTypeFlag::eBackbuffer;
			backbuffer_resource_binding.intermediate_state = gfx::ResourceStateFlag::eColorAttachmentWrite;
			backbuffer_resource_binding.final_state = gfx::ResourceStateFlag::ePresent;

			// Create default samplers

			gfx::Sampler* nearest_sampler{ nullptr };
			{
				gfx::SamplerCreateInfo create_info{};
				create_info.min_filter = gfx::Filter::eNearest;
				create_info.mag_filter = gfx::Filter::eNearest;
				create_info.mip_map_mode = gfx::Filter::eNearest;

				nearest_sampler = GetOrCreateSampler(create_info);
			}

			gfx::Sampler* linear_sampler{ nullptr };
			{
				gfx::SamplerCreateInfo create_info{};
				create_info.min_filter = gfx::Filter::eLinear;
				create_info.mag_filter = gfx::Filter::eLinear;
				create_info.mip_map_mode = gfx::Filter::eLinear;

				linear_sampler = GetOrCreateSampler(create_info);
			}

			// Load some static textures
			auto casual_day_4k_cube = AddRenderResource("/root/embed/rendering/textures/CasualDay4K.ktx2");
			auto dirt_mask_texture = AddRenderResource("/root/embed/rendering/textures/DirtMaskTextureExample.png");

			// GI targets
			uint32_t brdf_lut_target{ ~0u };
			{
				// TODO: make it fixed size
				gfx::TextureCreateInfo create_info{};
				create_info.width = create_info.height = 1024u;
				create_info.format = gfx::Format::eR16G16Sfloat;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eStorage;
				create_info.resizable = false;

				brdf_lut_target = AddRenderResource(create_info, nearest_sampler);
			}

			uint32_t irradiance_cube_target{ ~0u };
			{
				gfx::TextureCreateInfo create_info{};
				create_info.width = create_info.height = 64u;
				create_info.format = gfx::Format::eR16G16Sfloat;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eStorage | gfx::TextureUsageFlag::eCubemap;
				create_info.resizable = false;

				irradiance_cube_target = AddRenderResource(create_info, linear_sampler);
			}

			uint32_t prefiltered_cube_target{ ~0u };
			{
				gfx::TextureCreateInfo create_info{};
				create_info.width = create_info.height = 512u;
				create_info.format = gfx::Format::eR16G16B16A16Sfloat;
				create_info.usage = gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eStorage | gfx::TextureUsageFlag::eCubemap;
				create_info.levels = static_cast<uint32_t>(std::floor(std::log2((std::max)(create_info.width, create_info.height)))) + 1u;
				create_info.resizable = false;

				prefiltered_cube_target = AddRenderResource(create_info, linear_sampler);
			}
			// Passes
			// Generate brdflut
			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.shader_name = "lighting:brgflut_generator";
				create_info.one_time = true;
				create_info.BindWriteResource("out_color"_hash32, brdf_lut_target);

				create_info.draw_callback = [this, brdf_lut_target](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						self.FlushResources(command_list);

						uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };

						auto& resource = graph_resources[brdf_lut_target];
						auto* texture = GetTextureResource(resource);

						group_x = texture->GetWidth();
						group_y = texture->GetHeight();

						self.DivideWorkgroupByLocalSizes(group_x, group_y, group_z);
						command_list.Dispatch(group_x, group_y, group_z);
					};

				render_graph.emplace_back(CreateShaderPassNode(create_info));
			}

			// Generate irradiance cubemap
			{
				ComputeShaderPassCreateInfo create_info{};
				create_info.shader_name = "lighting:irradiancecube_generator";
				create_info.one_time = true;
				create_info.BindReadResource("sampler_color"_hash32, casual_day_4k_cube);
				create_info.BindWriteResource("out_color"_hash32, irradiance_cube_target);

				create_info.draw_callback = [this, irradiance_cube_target](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						self.FlushResources(command_list);

						uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };

						auto& resource = graph_resources[irradiance_cube_target];
						auto* texture = GetTextureResource(resource);

						group_x = texture->GetWidth();
						group_y = texture->GetHeight();

						self.DivideWorkgroupByLocalSizes(group_x, group_y, group_z);
						command_list.Dispatch(group_x, group_y, group_z);
					};

				render_graph.emplace_back(CreateShaderPassNode(create_info));
			}

			// Generate prefiltered cubemap
			{
				//auto* group_node = AllocateGroupNode();
				//auto* group = CastRenderGraphNode<RenderGraphGroupNode>(group_node);
				//group->group_name = "prefiltered map generator";
				//group->color = 0x99F9F1FF;
				//group->nodes = {};
				//group->nodes.reserve(10);

				auto& resource = graph_resources[prefiltered_cube_target];
				auto* texture = GetTextureResource(resource);

				for (int32_t mip = 0; mip < texture->GetMipLevelCount(); ++mip)
				{
					ComputeShaderPassCreateInfo create_info{};
					create_info.shader_name = "lighting:prefilteredmap_generator";
					create_info.one_time = true;
					create_info.BindWriteResource("out_color"_hash32, prefiltered_cube_target, mip);
					create_info.BindReadResource("sampler_color"_hash32, casual_day_4k_cube);

					create_info.draw_callback = [this, mip, texture](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
						{
							self.FlushResources(command_list);

							uint32_t group_x{ 1u }, group_y{ 1u }, group_z{ 1u };

							float layer_roughness = static_cast<float>(mip) / static_cast<float>(texture->GetMipLevelCount());
							
							auto& constants = self.GetConstantBlock();
							constants.Set("roughness"_hash32, layer_roughness);
							self.PushConstantRanges(command_list);

							group_x = texture->GetWidth();
							group_y = texture->GetHeight();

							self.DivideWorkgroupByLocalSizes(group_x, group_y, group_z);
							command_list.Dispatch(group_x, group_y, group_z);
						};

					render_graph.emplace_back(CreateShaderPassNode(create_info));
					//group->nodes.emplace_back(CreateShaderPassNode(create_info));
				}

				//render_graph.emplace_back(group_node);
			}

			// Test output shader
			{
				RenderShaderPassCreateInfo create_info{};
				create_info.shader_name = "testing:screenspace";
				create_info.BindReadResource("input_image"_hash32, brdf_lut_target);
				create_info.AddWriteAttachment(backbuffer_resource, ResourceAttachmentTypeFlag::eColor, gfx::ClearColor{ 1.f, 0.5f, 1.f, 1.f });

				create_info.draw_callback = [this, irradiance_cube_target](ShaderPass& self, gfx::CommandList& command_list, float delta_time)
					{
						auto& backbuffer = render_context->GetBackbuffer();

						auto viewport_width = backbuffer.GetWidth();
						auto viewport_height = backbuffer.GetHeight();

						command_list.SetViewport(0.f, 0.f, viewport_width, viewport_height, 0.f, 1.f);
						command_list.SerScissor(0u, 0u, viewport_width, viewport_height);
						
						self.FlushResources(command_list);
						
						command_list.Draw(3u, 0u, 0u, 1u);
					};

				render_graph.emplace_back(CreateShaderPassNode(create_info));
			}
		}

		void Renderer::BeginFrame()
		{
			shared_descriptor->Reset();

			bool surface_updated{ false };
			auto& command_list = render_context->Begin(surface_updated);
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

			command_list.ResetQuery(timestamp_query, 0u, 2u);
			command_list.BeginQuery(timestamp_query, 0u);

			global_resource_descriptor = shared_descriptor->RequestDescriptors();

			// Update bindless textures
			for (int32_t i = 0; i < textures.data().size(); ++i)
			{
				auto& resource = textures.Get(i);

				gfx::TextureDescriptor descriptor{};
				descriptor.type = gfx::DescriptorType::eCombinedImageSampler;
				descriptor.set = 0;
				descriptor.binding = 3;

				descriptor.array_index = i;
				descriptor.texture = resource.texture;
				descriptor.sampler = resource.sampler;

				global_resource_descriptor->BindTexture(descriptor);
			}

			global_resource_descriptor->Flush();
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

			for (auto& graph_resource : graph_resources)
			{
				if (graph_resource.resource_type & RenderResourceTypeFlag::eStaticImage)
					continue;

				graph_resource.current_state = gfx::ResourceStateFlag::eUndefined;
				for (auto& subresource_state : graph_resource.subresource_states)
				{
					subresource_state = gfx::ResourceStateFlag::eUndefined;
				}

				if ((graph_resource.resource_type & RenderResourceTypeFlag::eRenderTarget) || (graph_resource.resource_type & RenderResourceTypeFlag::eStorageTarget))
				{
					auto& resource = render_targets.Get(graph_resource.resource_id);
					resource.texture->Reset();
				}
			}
		}

		void Renderer::UpdateRenderTargets()
		{
			auto& swapchain = render_context->GetSwapchain();

			uint32_t width, height;
			swapchain.GetSurfaceExtent(width, height);

			// Update/resize render targets here if it's needed
			for (auto& target : render_targets.data())
			{
				// TODO: resize only if texture is able to resize 
				if (!target.resizable)
					continue;

				// Save pointer for delete old one
				auto* old_target = target.texture;

				gfx::TextureCreateInfo create_info{};
				create_info.usage = target.usage;
				create_info.width = width;
				create_info.height = height;
				create_info.layers = (target.usage & gfx::TextureUsageFlag::eCubemap) ? old_target->GetArrayLayerCount() / 6u : old_target->GetArrayLayerCount();
				create_info.levels = old_target->GetMipLevelCount();
				create_info.format = old_target->GetFormat();
				create_info.resizable = target.resizable;

				target.texture = gfx_context->CreateTexture(create_info);
				gfx_context->Destroy(old_target);
			}
		}
	}
}