#pragma once

#include <edge/util/utl_vector.h>
#include "IRenderer.h"

namespace e2d {
	class RendererFrame final : public IRendererFrame {
	public:
		~RendererFrame() override;

		static auto construct(const edge::gfx::IQueue& queue, const edge::gfx::ISwapchain& swapchain, uint32_t frame_index) -> std::unique_ptr<IRendererFrame>;

		void reset() override;
		void update(const edge::gfx::ISwapchain& swapchain) override;

		auto get_backbuffer() const -> edge::gfx::ITexture& override;

	private:
		edge::gfx::ITexture* backbuffer{ nullptr };
		edge::gfx::ISemaphore* image_avaliable{ nullptr };
		edge::gfx::ISemaphore* render_finished{ nullptr };
		edge::gfx::IFence* fence{ nullptr };
		uint32_t index{ ~0u };
	};

	class Renderer final : public IRenderer {
	public:
		Renderer(const edge::gfx::IQueue& queue);
		~Renderer() override;

		static auto construct(const RendererCreateInfo& create_info) -> std::unique_ptr<IRenderer>;

		void begin_frame(float delta_time) override;
		void end_frame(float delta_time) override;
	private:
		const edge::gfx::IQueue& graphics_queue;

		std::unique_ptr<edge::gfx::ISwapchain> swapchain;

		edge::utl::fixed_vector<std::unique_ptr<IRendererFrame>> frames_in_flight{};
	};
}