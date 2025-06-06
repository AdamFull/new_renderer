#include "Renderer.h"

#include <edge/gfx/gfx_context.h>

namespace e2d {
	// Renderer frame impl
	RendererFrame::~RendererFrame() {
		edge::gfx::destroy(backbuffer);
	}

	auto RendererFrame::construct(const edge::gfx::IQueue& queue, const edge::gfx::ISwapchain& swapchain,
		uint32_t frame_index) -> std::unique_ptr<IRendererFrame> {

		std::unique_ptr<RendererFrame> self{};



		return self;
	}

	void RendererFrame::reset() {

	}

	void RendererFrame::update(const edge::gfx::ISwapchain& swapchain) {
		edge::gfx::destroy(backbuffer);
		backbuffer = edge::gfx::create_swapchain_texture(swapchain, index);
	}

	auto RendererFrame::get_backbuffer() const -> edge::gfx::ITexture& {
		return *backbuffer;
	}

	Renderer::Renderer(const edge::gfx::IQueue& queue) :
		graphics_queue{ queue } {

	}

	Renderer::~Renderer() {
		graphics_queue.wait_idle();
	}

	auto Renderer::construct(const RendererCreateInfo& create_info) -> std::unique_ptr<IRenderer> {
		std::unique_ptr<Renderer> self{};

		auto* direct_queue = edge::gfx::get_queue(edge::gfx::QueueType::eDirect);
		self = std::make_unique<Renderer>(*direct_queue);

		// Create swapchain resource
		edge::gfx::SwapChainCreateInfo swapchain_ci{};
		swapchain_ci.window = create_info.window;
		swapchain_ci.image_count = 2u;

		auto swapchain = edge::gfx::create_swapchain(swapchain_ci);
		self->swapchain.reset(swapchain);

		auto image_count = swapchain->get_image_count();
		for (int32_t i = 0; i < static_cast<int32_t>(image_count); ++i) {
			self->frames_in_flight.emplace_back(RendererFrame::construct(self->graphics_queue, *self->swapchain, i));
		}

		return self;
	}

	void Renderer::begin_frame(float delta_time) {

	}

	void Renderer::end_frame(float delta_time) {

	}
}