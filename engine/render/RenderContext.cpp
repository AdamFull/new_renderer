#include <engine/render/RenderContext.h>

#include <engine/runtime/Window.h>

#include <engine/render/render_api/GFX.h>

namespace engine
{
	namespace render
	{
		RenderContext::RenderContext(gfx::Context& context, Window* window) :
			context{ context },
			window{ *window }
		{
			// Select queue that can present
			queue = context.RequestQueue(gfx::QueueTypeFlag::ePresent);
			
			gfx::SwapChainCreateInfo swapchain_ci{};
			swapchain_ci.window = window;
			swapchain_ci.image_count = 2u;

			swapchain = context.CreateSwapchain(swapchain_ci);

			auto image_count = swapchain->GetImageCount();

			for (int32_t i = 0; i < image_count; ++i)
			{
				render_frames[i] = std::make_unique<RenderFrame>(context, *queue);

				gfx::Texture* backbuffer = context.CreateSwapchainTexture(swapchain, i);
				render_frames[i]->UpdateBackbuffer(backbuffer);
			}
		}

		RenderContext::~RenderContext()
		{

		}
	}
}