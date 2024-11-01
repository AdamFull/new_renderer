#include "RenderGraph.h"
#include <engine/rendering/RendererInterface.h>
#include <engine/rendering/RenderContext.h>
#include <engine/rendering/gfx/Context.h>

#include "RenderPass.h"
#include "ComputePass.h"

namespace engine
{
	namespace render
	{
		GroupNode::GroupNode(const std::string& name, uint32_t color) :
			group_name{ name },
			group_color{ color }
		{
		}

		void GroupNode::AddNode(std::unique_ptr<IGraphNode>&& node)
		{
			graph_nodes.emplace_back(std::move(node));
		}

		void GroupNode::Evaluate(gfx::CommandList& command_list, float delta_time)
		{
			command_list.BeginMarker(group_name.c_str(), group_color);
			
			for (auto& node : graph_nodes)
				node->Evaluate(command_list, delta_time);
			
			command_list.EndMarker();
		}


	
		RenderGraph::RenderGraph(IRenderer& renderer, Window* window) :
			renderer{ renderer },
			context{ renderer.GetContext() },
			resource_context{ renderer .GetResourceContext() }
		{
			render_context = std::make_unique<RenderContext>(context, window);

			// Create default image sampler
			gfx::SamplerCreateInfo sampler_ci{};
			sampler_ci.address_u = sampler_ci.address_v = sampler_ci.address_w = gfx::SamplerAddressMode::eClampToEdge;
			sampler_ci.min_filter = gfx::Filter::eLinear;
			sampler_ci.mag_filter = gfx::Filter::eLinear;
			sampler_ci.mip_map_mode = gfx::Filter::eLinear;

			default_sampler = context.CreateSampler(sampler_ci);

			// Create timestamp query for collecting pass info
			gfx::QueryPoolCreateInfo query_pool_ci{};
			query_pool_ci.type = gfx::QueryType::eTimestamp;
			query_pool_ci.query_count = 2u;
			timestamp_query =context.CreateQueryPool(query_pool_ci);
			timestamp_query->Reset(0u);

			context.RequestTimestampFrequency(&timestamp_frequency);

			// TODO: now it creates just default one
			gfx::SharedDescriptorCreateInfo shared_descriptor_ci{};
			shared_descriptor_ci.descriptions.emplace_back(gfx::DescriptorType::eUniformBuffer, 0u, 1u);
			shared_descriptor_ci.descriptions.emplace_back(gfx::DescriptorType::eUniformBuffer, 1u, 1u);
			shared_descriptor_ci.descriptions.emplace_back(gfx::DescriptorType::eUniformBuffer, 2u, 1u);
			shared_descriptor_ci.descriptions.emplace_back(gfx::DescriptorType::eCombinedImageSampler, 3u, 16384u);

			shared_descriptor = context.CreateSharedDescriptor(shared_descriptor_ci);
		}

		RenderGraph::~RenderGraph()
		{
			context.Destroy(default_sampler);
			context.Destroy(shared_descriptor);
			context.Destroy(timestamp_query);
		}

		uint32_t RenderGraph::AddResource()
		{
			// Just creates empty resource)
			auto& new_resource = render_resources.emplace_back();
			new_resource.subresource_states.fill({});
			return render_resources.size() - 1ull;
		}

		uint32_t RenderGraph::AddResource(const std::string& path, gfx::Sampler* sampler)
		{
			auto& new_resource = render_resources.emplace_back();
			new_resource.resource_type = RenderResourceTypeFlag::eStaticImage;

			gfx::Texture* texture = resource_context.LoadTexture(path);
			new_resource.resource_id = static_textures.Add(TextureObject{ texture, sampler ? sampler : default_sampler });

			new_resource.current_state.flags = gfx::ResourceStateFlag::eShaderResource;

			return render_resources.size() - 1ull;
		}

		uint32_t RenderGraph::AddResource(const gfx::TextureCreateInfo& create_info, gfx::Sampler* sampler)
		{
			auto& new_resource = render_resources.emplace_back();

			// TODO: What if it's storage target and render target at the same time?
			if (create_info.usage & gfx::TextureUsageFlag::eStorage)
			{
				new_resource.intermediate_state = gfx::ResourceStateFlag::eComputeWrite;
				new_resource.final_state_comp = gfx::ResourceStateFlag::eComputeRead;
				new_resource.final_state_other = gfx::ResourceStateFlag::eShaderResource;
				new_resource.resource_type = RenderResourceTypeFlag::eStorageTarget;
			}
			else if (create_info.usage & gfx::TextureUsageFlag::eRenderTarget)
			{
				auto is_depth = IsDepthStencilFormat(create_info.format);
				new_resource.intermediate_state = is_depth ? gfx::ResourceStateFlag::eDepthWrite : gfx::ResourceStateFlag::eAttachmentWrite;
				new_resource.final_state_comp = is_depth ? gfx::ResourceStateFlag::eDepthRead : gfx::ResourceStateFlag::eComputeRead;
				new_resource.final_state_other = is_depth ? gfx::ResourceStateFlag::eDepthRead : gfx::ResourceStateFlag::eShaderResource;
				new_resource.resource_type = RenderResourceTypeFlag::eRenderTarget;
			}
			else
				new_resource.resource_type = RenderResourceTypeFlag::eStaticImage;

			gfx::Texture* texture = context.CreateTexture(create_info);
			if ((create_info.usage & gfx::TextureUsageFlag::eRenderTarget) || (create_info.usage & gfx::TextureUsageFlag::eStorage))
				new_resource.resource_id = render_targets.Add(TextureObject{ texture, sampler ? sampler : default_sampler, create_info.usage, create_info.resizable });
			else
				new_resource.resource_id = static_textures.Add(TextureObject{ texture, sampler ? sampler : default_sampler, create_info.usage, create_info.resizable });

			new_resource.subresource_states.fill({});

			return render_resources.size() - 1ull;
		}

		uint32_t RenderGraph::AddResource(const gfx::BufferCreateInfo& create_info)
		{
			gfx::Buffer* buffer = context.CreateBuffer(create_info);
			auto handle = buffers.Add(BufferObject{ buffer });
			if (create_info.usage != gfx::BufferUsageFlag::eStagingBuffer && create_info.usage != gfx::BufferUsageFlag::eUniformBuffer)
			{
				auto& new_resource = render_resources.emplace_back();
				new_resource.resource_type = RenderResourceTypeFlag::eBuffer;
				new_resource.resource_id = handle;

				new_resource.intermediate_state = gfx::ResourceStateFlag::eComputeWrite;
				new_resource.final_state_comp = gfx::ResourceStateFlag::eComputeRead;
				new_resource.final_state_other = gfx::ResourceStateFlag::eShaderResource;

				return render_resources.size() - 1ull;
			}

			return ~0u;
		}

		RenderGraphResource& RenderGraph::GetResource(uint32_t resource_index)
		{
			return render_resources[resource_index];
		}

		RenderGraphResource const& RenderGraph::GetResource(uint32_t resource_index) const
		{
			return render_resources[resource_index];
		}

		std::unique_ptr<IGraphNode> RenderGraph::CreateNode(const std::string& group_name, uint32_t group_color)
		{
			return std::make_unique<GroupNode>(group_name, group_color);
		}

		std::unique_ptr<IGraphNode> RenderGraph::CreateNode(const RenderShaderPassCreateInfo& create_info)
		{
			RenderPassCreateInfo pass_ci{};
			pass_ci.debug_name = create_info.shader_name;
			pass_ci.shader = resource_context.LoadShader(create_info.shader_name);
			pass_ci.shared_descriptor = shared_descriptor;
			pass_ci.primitive_topology = gfx::PrimitiveTopology::eTriangleList;
			pass_ci.draw_callback = std::move(create_info.draw_callback);
			pass_ci.resource_attachments = create_info.resource_attachments;
			//pass_ci.state.rasterization_state.front_face = gfx::FrontFace::eCW;

			auto new_pass = std::make_unique<RenderPass>(*this, pass_ci);
			new_pass->SetExecutionContext(render_context.get());
			return new_pass;
		}

		std::unique_ptr<IGraphNode> RenderGraph::CreateNode(const ComputeShaderPassCreateInfo& create_info)
		{
			ComputePassCreateInfo pass_ci{};
			pass_ci.debug_name = create_info.shader_name;
			pass_ci.shader = resource_context.LoadShader(create_info.shader_name);
			pass_ci.one_time = create_info.one_time;
			pass_ci.shared_descriptor = shared_descriptor;
			pass_ci.draw_callback = std::move(create_info.draw_callback);

			auto new_pass = std::make_unique<ComputePass>(*this, pass_ci);
			new_pass->SetExecutionContext(render_context.get());
			return new_pass;
		}

		void RenderGraph::AddNode(std::unique_ptr<IGraphNode>&& node)
		{
			graph_nodes.emplace_back(std::move(node));
		}

		void RenderGraph::Evaluate(float delta_time)
		{
			auto& command_list = render_context->GetCurrentCommandList();

			for (auto& node : graph_nodes)
				node->Evaluate(command_list, delta_time);

			gfx::TextureBarrier texture_barrier{};
			gfx::ResourceBarrierInfo resource_barriers{};
			resource_barriers.texture_barriers = &texture_barrier;
			resource_barriers.texture_barrier_count = 1;

			texture_barrier.texture = &render_context->GetBackbuffer();
			// TODO: get current resource state from resource itself
			texture_barrier.src_state = gfx::ResourceStateFlag::eAttachmentWrite;
			texture_barrier.dst_state = gfx::ResourceStateFlag::ePresent;

			command_list.ResourceBarrier(resource_barriers);
		}

		void RenderGraph::BeginFrame()
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
			uint32_t array_index{ 0u };
			for (auto& resource : static_textures.Data())
			{
				gfx::TextureDescriptor descriptor{};
				descriptor.type = gfx::DescriptorType::eCombinedImageSampler;
				descriptor.set = 0;
				descriptor.binding = 3;
			
				descriptor.array_index = array_index++;
				descriptor.texture = resource.texture;
				descriptor.sampler = resource.sampler;
				descriptor.resource_state = gfx::ResourceStateFlag::eShaderResource;
			
				global_resource_descriptor->BindTexture(descriptor);
			}
			
			global_resource_descriptor->Flush();
		}

		void RenderGraph::EndFrame()
		{
			gfx::SemaphoreSubmitInfo upload_wait_semaphore{};
			//auto upload_avaliable = render_context->GetUploadAvaliableSemaphore();
			//upload_wait_semaphore = upload_context->Submit(&upload_avaliable, upload_avaliable.semaphore ? 1 : 0);
			upload_wait_semaphore = resource_context.Submit(nullptr, 0);

			auto& command_list = render_context->GetCurrentCommandList();
			command_list.EndQuery(timestamp_query, 1u);

			render_context->EndFrame(upload_wait_semaphore);

			for (auto& render_resource : render_resources)
			{
				if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage)
					continue;

				render_resource.current_state.Reset();
				for (auto& subresource_state : render_resource.subresource_states)
				{
					subresource_state.Reset();
				}

				if ((render_resource.resource_type & RenderResourceTypeFlag::eRenderTarget) || (render_resource.resource_type & RenderResourceTypeFlag::eStorageTarget))
				{
					auto& resource = render_targets.Get(render_resource.resource_id);
					resource.texture->Reset();
				}
			}
		}

		void RenderGraph::UpdateRenderTargets()
		{
			auto& swapchain = render_context->GetSwapchain();

			uint32_t width, height;
			swapchain.GetSurfaceExtent(width, height);

			// Update/resize render targets here if it's needed
			for (auto& target : render_targets.Data())
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

				target.texture = context.CreateTexture(create_info);
				context.Destroy(old_target);
			}
		}

		gfx::Context& RenderGraph::GetContext()
		{
			return context;
		}

		gfx::Context const& RenderGraph::GetContext() const
		{
			return context;
		}

		gfx::Texture* RenderGraph::GetTexture(uint32_t resource_index)
		{
			auto& render_resource = render_resources[resource_index];
			return GetTexture(render_resource);
		}

		gfx::Texture* RenderGraph::GetTexture(const RenderGraphResource& render_resource)
		{
			if (render_resource.resource_type & RenderResourceTypeFlag::eBackbuffer)
			{
				auto& backbuffer = render_context->GetBackbuffer();
				return &backbuffer;
			}
			else if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage)
			{
				auto& texture_object = static_textures.Get(render_resource.resource_id);
				return texture_object.texture;
			}
			else if (render_resource.resource_type & RenderResourceTypeFlag::eAllImage)
			{
				auto& texture_object = render_targets.Get(render_resource.resource_id);
				return texture_object.texture;
			}

			return nullptr;
		}

		gfx::Sampler* RenderGraph::GetSampler(uint32_t resource_index)
		{
			auto& render_resource = render_resources[resource_index];
			return GetSampler(render_resource);
		}

		gfx::Sampler* RenderGraph::GetSampler(const RenderGraphResource& render_resource)
		{
			if (render_resource.resource_type & RenderResourceTypeFlag::eBackbuffer)
			{
				return default_sampler;
			}
			else if (render_resource.resource_type & RenderResourceTypeFlag::eStaticImage)
			{
				auto& texture_object = static_textures.Get(render_resource.resource_id);
				return texture_object.sampler;
			}
			else if (render_resource.resource_type & RenderResourceTypeFlag::eAllImage)
			{
				auto& texture_object = render_targets.Get(render_resource.resource_id);
				return texture_object.sampler;
			}

			return default_sampler;
		}

		gfx::Buffer* RenderGraph::GetBuffer(uint32_t resource_index)
		{
			auto& render_resource = render_resources[resource_index];
			return GetBuffer(render_resource);
		}

		gfx::Buffer* RenderGraph::GetBuffer(const RenderGraphResource& render_resource)
		{
			if (render_resource.resource_type & RenderResourceTypeFlag::eBuffer)
			{
				auto& buffer_object = buffers.Get(render_resource.resource_id);
				return buffer_object.buffer;
			}

			return nullptr;
		}
	}
}