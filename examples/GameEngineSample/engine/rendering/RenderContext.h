#pragma once

#include <future>

#include "RenderResourceContainer.h"

#include "ExecutionContext.h"
#include <span>

#include <edge/gfx/gfx_base.h>

namespace edge {
	class Window;

	namespace gfx {
		class TechniqueDatabase;
	}
}

namespace engine::render {
	class IRenderer;
	class UniformPool;

	class RenderContext final : public IExecutionContext {
	public:
		RenderContext(edge::Window* window);
		~RenderContext();

		RenderContext(const RenderContext&) = delete;
		RenderContext(RenderContext&&) = delete;
		RenderContext& operator=(const RenderContext&) = delete;
		RenderContext& operator=(RenderContext&&) = delete;

		auto add_resource() -> uint32_t override;
		auto add_resource_sync(std::string_view path) -> uint32_t override;
		auto add_resource(std::string_view path) -> uint32_t override;
		auto add_resource(const TextureResourceCreateInfo& create_info, std::span<uint8_t> texture_data) -> uint32_t override;
		auto add_resource(const TextureResourceCreateInfo& create_info) -> uint32_t override;
		auto add_resource(const edge::gfx::BufferCreateInfo& create_info) -> uint32_t override;
		auto add_resource(const edge::gfx::SamplerCreateInfo& create_info) -> uint32_t override;

		void set_resource_name(uint32_t resource_index, std::string_view name) override;

		auto get_resource(uint32_t resource_index) -> RenderResource& override;
		auto get_resource(uint32_t resource_index) const -> RenderResource const&override;

		auto create_node(std::string_view group_name, uint32_t group_color = 0xFFFFFFFF) -> std::unique_ptr<IExecutionNode> override;
		auto create_node(const RenderShaderPassCreateInfo& create_info) -> std::unique_ptr<IExecutionNode> override;
		auto create_node(const ComputeShaderPassCreateInfo& create_info) -> std::unique_ptr<IExecutionNode> override;

		void add_node(std::unique_ptr<IExecutionNode>&& node) override;
		void execute(float delta_time) override;

		void begin() override;
		void end() override;

		auto get_texture(uint32_t resource_index) -> edge::gfx::ITexture* override;
		auto get_texture(const RenderResource& render_resource) -> edge::gfx::ITexture* override;

		auto get_sampler(uint32_t resource_index) -> edge::gfx::ISampler* override;
		auto get_sampler(const RenderResource& render_resource) -> edge::gfx::ISampler* override;

		auto get_buffer(uint32_t resource_index) -> edge::gfx::IBuffer* override;
		auto get_buffer(const RenderResource& render_resource) -> edge::gfx::IBuffer* override;

		auto request_uniform_block(uint64_t block_size) -> edge::gfx::BufferSuballocation override;
		auto get_current_command_list() -> edge::gfx::ICommandList& override;

		auto get_binding_layouts(std::span<edge::gfx::IDescriptorLayout*> binding_layouts) -> std::vector<edge::gfx::IBindingLayout*> override;
	protected:
		void update_global_binding(uint32_t binding, std::span<const uint8_t> data);

		auto get_current_frame_index() const -> const uint32_t&;

		void update_swapchain(uint32_t width, uint32_t height);
		void update_swapchain(uint32_t image_count);

		bool handle_surface_change(bool force_update = false);

		class RenderFrame;
		void update_render_targets();
		void update_resources();
	private:
		const edge::Window& window;
		const edge::gfx::IQueue* queue;

		std::unique_ptr<edge::gfx::TechniqueDatabase> technique_db{};

		edge::gfx::ISwapchain* swapchain{ nullptr };
		//vk::SurfaceTransformFlagBitsKHR pre_transform{ vk::SurfaceTransformFlagBitsKHR::eIdentity };

		edge::gfx::ITexture* dummy_texture{ nullptr };
		std::vector<std::pair<std::shared_ptr<edge::gfx::AsyncResourceHandle>, uint32_t>> pending_resources;

		RenderFrame* current_frame{ nullptr };
		edge::gfx::IDescriptorLayout* global_descriptor{ nullptr };

		static constexpr size_t k_max_frames_in_flight{ 8ull };
		std::array<std::unique_ptr<RenderFrame>, k_max_frames_in_flight> render_frames{};
		uint32_t active_frame_index{ 0u };
		bool frame_active{ false };

		edge::gfx::ISemaphore* acquired_semaphore{ nullptr };
		edge::gfx::ISemaphore* previous_frame_semaphore{ nullptr };
		edge::gfx::ICommandList* current_command_list{ nullptr };

		std::vector<std::unique_ptr<IExecutionNode>> graph_nodes{};
		std::vector<RenderResource> render_resources{};

		edge::gfx::ISampler* default_sampler{ nullptr };

		// Performance counters
		edge::gfx::IQueryPool* timestamp_query{ nullptr };
		double timestamp_frequency{ 0.f };
		double gpu_delta_time{ 0.0 };

		// Shared descriptors
		std::unique_ptr<UniformPool> global_uniform_pool{};

		ResourceContainer<TextureObject> static_textures{};
		ResourceContainer<TextureObject> render_targets{};
		ResourceContainer<edge::gfx::IBuffer*> buffers{};
		ResourceContainer<edge::gfx::ISampler*> samplers{};
	};
}