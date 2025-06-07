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
		auto get_or_allocate_binding_layout(edge::gfx::IDescriptorLayout& descriptor_layout) -> edge::gfx::IBindingLayout & override;
	private:
		void create_resources(const edge::gfx::IQueue& queue, const edge::gfx::ISwapchain& swapchain);
	private:
		std::unique_ptr<edge::gfx::ITexture> backbuffer;
		uint32_t index{ ~0u };

		std::unique_ptr<edge::gfx::ICommandPool> command_pool;
		std::unique_ptr<edge::gfx::ISemaphore> image_avaliable;
		std::unique_ptr<edge::gfx::ISemaphore> render_finished;
		std::unique_ptr<edge::gfx::IFence> fence;

		std::unique_ptr<edge::gfx::IDescriptorAllocator> descriptor_allocator;
		std::unordered_map<uint32_t, std::vector<std::unique_ptr<edge::gfx::IBindingLayout>>> binding_layout_table{};
	};

	class Renderer final : public IRenderer {
	public:
		Renderer(const edge::gfx::IQueue& queue);
		~Renderer() override;

		static auto construct(const RendererCreateInfo& create_info) -> std::unique_ptr<IRenderer>;

		void begin_frame(float delta_time) override;
		void end_frame(float delta_time) override;

		void on_surface_update() override;
	private:
		void create_resources(const edge::Window& window);

		void update_swapchain(uint32_t width, uint32_t height);
		void update_swapchain(uint32_t image_count);
		bool handle_surface_change(bool force = false);
	private:
		const edge::Window* window{ nullptr };
		const edge::gfx::IQueue& graphics_queue;

		std::unique_ptr<edge::gfx::ISwapchain> swapchain;

		bool frame_active{ false };
		int32_t active_frame_index{ 0 };
		IRendererFrame* current_frame{ nullptr };
		edge::utl::fixed_vector<std::unique_ptr<IRendererFrame>> frames_in_flight{};
	};
}