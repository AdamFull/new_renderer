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


	Renderer::~Renderer() {

	}

	auto Renderer::construct(const RendererCreateInfo& create_info) -> std::unique_ptr<IRenderer> {
		std::unique_ptr<Renderer> self{};



		return self;
	}
}