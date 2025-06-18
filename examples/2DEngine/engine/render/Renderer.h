#pragma once

#include <edge/gfx/gfx_uniform_pool.h>
#include <edge/util/utl_vector.h>
#include "IRenderer.h"

#include <edge/gfx/technique_system/gfx_technique_system.h>

namespace e2d {
	class RendererFrame final : public IRendererFrame {
	public:
		~RendererFrame() override;

		static auto construct(const edge::gfx::IQueue& queue, const edge::gfx::ISwapchain& swapchain, uint32_t frame_index) -> std::unique_ptr<IRendererFrame>;

		void reset() override;
		void update(const edge::gfx::ISwapchain& swapchain) override;
		void begin() override;
		void end() override;

		auto get_fence() -> edge::gfx::IFence& override;
		auto get_command_list() -> edge::gfx::ICommandList& override;
		auto get_acquired_semaphore() -> edge::gfx::ISemaphore& override;
		auto get_ready_semaphore() -> edge::gfx::ISemaphore& override;
		auto get_backbuffer() const -> edge::gfx::ITexture& override;
		auto get_or_allocate_binding_layout(edge::gfx::IDescriptorLayout& descriptor_layout) -> edge::gfx::IBindingLayout& override;
		auto allocate_uniform_block(uint64_t size) -> edge::gfx::BufferSuballocation override;
	private:
		void create_resources(const edge::gfx::IQueue& queue, const edge::gfx::ISwapchain& swapchain);
	private:
		std::unique_ptr<edge::gfx::ITexture> backbuffer;
		uint32_t index{ ~0u };

		std::unique_ptr<edge::gfx::IUniformPool> uniform_pool;

		std::unique_ptr<edge::gfx::ICommandPool> command_pool;
		edge::gfx::ICommandList* command_list{};

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

		auto get_binding_layout(edge::gfx::IDescriptorLayout* descriptor_layout) -> edge::gfx::IBindingLayout& override;
		auto get_backbuffer_resource() const noexcept -> RGResourceHandle override;
		auto get_render_graph() -> IRenderGraph& override;

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

		std::unique_ptr<edge::gfx::IQueryPool> query_pool;
		double timestamp_frequency{ 1.0 };
		double gpu_delta_time{ 0.0 };

		edge::gfx::ISemaphore* acquired_semaphore{ nullptr };

		bool frame_active{ false };
		uint32_t active_frame_index{ 0 };
		IRendererFrame* current_frame{ nullptr };
		edge::utl::fixed_vector<std::unique_ptr<IRendererFrame>> frames_in_flight{};

		std::unique_ptr<edge::gfx::TechniqueDatabase> technique_system_{};
		std::unique_ptr<IRenderGraph> render_graph_{};

		RGResourceHandle backbuffer_handle_;
	};
}