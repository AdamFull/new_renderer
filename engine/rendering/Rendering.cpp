#include <engine/rendering/Rendering.h>

#include <engine/rendering/gfx/Context.h>

#include <engine/rendering/RenderContext.h>
#include <engine/rendering/UploadContext.h>

namespace engine
{
	namespace render
	{
		Renderer::Renderer(Window* window)
		{
			gfx::ContextCreateInfo gfx_create_info{};
			gfx_create_info.app_name = "MyVulkanEngine";
			gfx_create_info.render_api = gfx::RenderApi::eVulkan;
			gfx_create_info.window = window;

			gfx_context = gfx::CreateContext(gfx_create_info);

			render_context = std::make_unique<RenderContext>(*gfx_context, window);
			upload_context = std::make_unique<UploadContext>(*gfx_context);

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
			sampler_ci.mip_map_mode = gfx::SamplerMipmapMode::eLinear;

			default_sampler = gfx_context->CreateSampler(sampler_ci);
		}

		Renderer::~Renderer()
		{
			gfx_context->Destroy(default_sampler);
			gfx_context->Destroy(timestamp_query);
		}

		void Renderer::Render(float delta_time)
		{
			BeginFrame();

			// DO RENDER

			EndFrame();
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
			upload_wait_semaphore = upload_context->Submit(nullptr, 0);

			render_context->EndFrame(upload_wait_semaphore);
		}

		void Renderer::UpdateRenderTargets()
		{

		}
	}
}