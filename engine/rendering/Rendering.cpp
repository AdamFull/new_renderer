#include <engine/rendering/Rendering.h>

#include <engine/rendering/gfx/Context.h>

#include <engine/rendering/RenderContext.h>
#include <engine/rendering/UploadContext.h>

#include <engine/rendering/render_graph/ComputePass.h>
#include <engine/rendering/render_graph/RenderPass.h>

#include <util/Logger.h>

namespace engine
{
	namespace render
	{
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
			query_pool_ci.query_count = 256;
			timestamp_query = gfx_context->CreateQueryPool(query_pool_ci);

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
			auto handle = textures.Add(TextureObject{ texture, sampler ? sampler : default_sampler });
			new_resource.resource_id = handle.Get();

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
				auto handle = render_targets.Add(TextureObject{ texture, sampler ? sampler : default_sampler });
				new_resource.resource_id = handle.Get();
			}

			auto handle = textures.Add(TextureObject{ texture, sampler ? sampler : default_sampler });
			new_resource.resource_id = handle.Get();

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
				new_resource.resource_id = handle.Get();

				return graph_resources.size() - 1ull;
			}

			return ~0u;
		}

		uint32_t Renderer::AddShaderPass(const ComputeShaderPassCreateInfo& create_info)
		{
			uint32_t pass_index = shader_passes.size();

			ComputePassCreateInfo pass_ci{};
			pass_ci.shader = resource_context->LoadShader(create_info.shader_name);

			auto& new_pass = shader_passes.emplace_back(std::make_unique<ComputePass>(*gfx_context, pass_ci));
			new_pass->SetExecutionContext(render_context.get());

			return pass_index;
		}

		uint32_t Renderer::AddShaderPass(const RenderShaderPassCreateInfo& create_info)
		{
			uint32_t pass_index = shader_passes.size();

			RenderPassCreateInfo pass_ci{};
			pass_ci.shader = resource_context->LoadShader(create_info.shader_name);

			auto& new_pass = shader_passes.emplace_back(std::make_unique<RenderPass>(*gfx_context, pass_ci));
			new_pass->SetExecutionContext(render_context.get());

			return pass_index;
		}

		void Renderer::Render(float delta_time)
		{
			BeginFrame();

			// Execute render graph
			EvaluateRenderGraph(delta_time);

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

		Handle Renderer::LoadTexture(const std::string& path, gfx::Sampler* sampler)
		{
			gfx::Texture* texture = resource_context->LoadTexture(path);
			return textures.Add(TextureObject{ texture, sampler ? sampler : default_sampler });
		}

		Handle Renderer::CreateTexture(const gfx::TextureCreateInfo& create_info, gfx::Sampler* sampler)
		{
			gfx::Texture* texture = gfx_context->CreateTexture(create_info);

			if(create_info.usage & gfx::TextureUsageFlag::eRenderTarget)
				return render_targets.Add(TextureObject{ texture, sampler ? sampler : default_sampler });

			return textures.Add(TextureObject{ texture, sampler ? sampler : default_sampler });
		}

		void Renderer::DestroyTexture(Handle handle)
		{
			// TODO: IMPLEMENT TEXTURE DELETION
			assert(false && "NOT IMPLEMENTED");
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

			gfx::ResourceBarrierInfo resource_barriers{};
			resource_barriers.buffer_barriers = buffer_barriers.data();
			resource_barriers.texture_barriers = texture_barriers.data();

			auto& shader_pass = shader_passes[pass_index];
			auto& pass_resouces = per_pass_resouce_access[pass_index];

			for (int32_t resource_access_type_i = 0; resource_access_type_i < 3ull; ++resource_access_type_i)
			{

			}

			// TODO: update attachments for render pass
			// TODO: execute barriers
		}

		void Renderer::EvaluateRenderGraph(float delta_time)
		{
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

					EvaluateShaderPass(pass_node->pass_index);
					pass->Execute(delta_time);
				} break;
				}
			}
		}


		void Renderer::BuildGraph()
		{
			

			RenderShaderPassCreateInfo create_info{};
			create_info.shader_name = "testing:screenspace";

			uint32_t test_shader_pass = AddShaderPass(create_info);
		}

		void Renderer::BeginFrame()
		{
			bool surface_updated{ false };
			auto& command_buffer = render_context->Begin(surface_updated);
			if (surface_updated)
			{
				UpdateRenderTargets();
			}

			timestamp_query->Reset(0u);
			query_index = 0u;

			// Update texture array
			for (auto& texture : textures.data())
			{
				// TODO: update here
			}
		}

		void Renderer::EndFrame()
		{
			gfx::SemaphoreSubmitInfo upload_wait_semaphore{};
			//auto upload_avaliable = render_context->GetUploadAvaliableSemaphore();
			//upload_wait_semaphore = upload_context->Submit(&upload_avaliable, upload_avaliable.semaphore ? 1 : 0);
			upload_wait_semaphore = resource_context->Submit(nullptr, 0);

			render_context->EndFrame(upload_wait_semaphore);

			// Reset local render targets states
			for (auto& target : render_targets.data())
			{
				target.texture->Reset();
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