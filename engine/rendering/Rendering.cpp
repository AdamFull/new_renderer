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
			gfx_context->Destroy(default_sampler);
			gfx_context->Destroy(timestamp_query);
		}

		uint32_t Renderer::AddShaderPass(const std::string& shader_name)
		{
			uint32_t pass_index = shader_passes.size();

			auto* shader = resource_context->LoadShader(shader_name);

			auto& new_pass = shader_passes.emplace_back(std::make_unique<RenderPass>(*gfx_context, shader));

			return pass_index;
		}

		void Renderer::Render(float delta_time)
		{
			BeginFrame();

			// DO RENDER

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
			return render_targets.Add(TextureObject{ texture, sampler ? sampler : default_sampler });
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

		void Renderer::BuildGraph()
		{
			uint32_t test_shader_pass = AddShaderPass("testing:screenspace");
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
		}

		void Renderer::EndFrame()
		{
			gfx::SemaphoreSubmitInfo upload_wait_semaphore{};
			//auto upload_avaliable = render_context->GetUploadAvaliableSemaphore();
			//upload_wait_semaphore = upload_context->Submit(&upload_avaliable, upload_avaliable.semaphore ? 1 : 0);
			upload_wait_semaphore = resource_context->Submit(nullptr, 0);

			render_context->EndFrame(upload_wait_semaphore);
		}

		void Renderer::UpdateRenderTargets()
		{

		}
	}
}