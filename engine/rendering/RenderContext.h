#pragma once

#include "RenderResourceContainer.h"

#include "ExecutionContext.h"
#include <span>

#include <edge/gfx/gfx_base.h>

namespace edge
{
	class Window;

	namespace gfx
	{
		namespace ext
		{
			class ShaderEffectSystem;
		}
	}
}

namespace engine
{
	namespace render
	{
		class IRenderer;
		class UniformPool;

		class RenderContext final : public ExecutionContext
		{
		public:
			RenderContext(IRenderer& renderer, edge::Window* window);
			~RenderContext();

			RenderContext(const RenderContext&) = delete;
			RenderContext(RenderContext&&) = delete;
			RenderContext& operator=(const RenderContext&) = delete;
			RenderContext& operator=(RenderContext&&) = delete;

			uint32_t AddResource() override;
			uint32_t AddResource(std::string_view path, edge::gfx::ISampler* sampler = nullptr) override;
			uint32_t AddResource(const TextureResourceCreateInfo& create_info, std::span<uint8_t> texture_data) override;
			uint32_t AddResource(const TextureResourceCreateInfo& create_info, edge::gfx::ISampler* sampler = nullptr) override;
			uint32_t AddResource(const edge::gfx::BufferCreateInfo& create_info) override;

			void SetResourceName(uint32_t resource_index, std::string_view name) override;

			RenderResource& GetResource(uint32_t resource_index) override;
			RenderResource const& GetResource(uint32_t resource_index) const override;

			std::unique_ptr<ExecutionNode> CreateNode(std::string_view group_name, uint32_t group_color = 0xFFFFFFFF) override;
			std::unique_ptr<ExecutionNode> CreateNode(const RenderShaderPassCreateInfo& create_info) override;
			std::unique_ptr<ExecutionNode> CreateNode(const ComputeShaderPassCreateInfo& create_info) override;

			void AddNode(std::unique_ptr<ExecutionNode>&& node) override;
			void Execute(float delta_time) override;

			void Begin() override;
			void End() override;

			edge::gfx::IContext& GetContext() override;
			edge::gfx::IContext const& GetContext() const override;

			edge::gfx::ITexture* GetTexture(uint32_t resource_index) override;
			edge::gfx::ITexture* GetTexture(const RenderResource& render_resource) override;

			edge::gfx::ISampler* GetSampler(uint32_t resource_index) override;
			edge::gfx::ISampler* GetSampler(const RenderResource& render_resource) override;

			edge::gfx::IBuffer* GetBuffer(uint32_t resource_index) override;
			edge::gfx::IBuffer* GetBuffer(const RenderResource& render_resource) override;

			edge::gfx::BufferSuballocation RequestUniformBlock(uint64_t block_size) override;
			edge::gfx::ICommandList& GetCurrentCommandList() override;
			edge::gfx::IDescriptorAllocator& GetCurrentDescriptorAllocator() override;
		protected:
			void UpdateGlobalBinding(uint32_t binding, std::span<const uint8_t> data);

			const uint32_t& GetCurrentFrameIndex() const;

			void UpdateSwapchain(uint32_t width, uint32_t height);
			void UpdateSwapchain(uint32_t image_count);

			bool HandleSurfaceChange(bool force_update = false);

			class RenderFrame;
			void UpdateRenderTargets();
			void UpdateResources();
		private:
			edge::gfx::IContext& context;
			edge::gfx::ext::ShaderEffectSystem& shader_effect_system;

			const edge::Window& window;
			const edge::gfx::IQueue* queue;

			edge::gfx::ISwapchain* swapchain{ nullptr };
			//vk::SurfaceTransformFlagBitsKHR pre_transform{ vk::SurfaceTransformFlagBitsKHR::eIdentity };

			RenderFrame* current_frame{ nullptr };

			static constexpr size_t k_max_frames_in_flight{ 8ull };
			std::array<std::unique_ptr<RenderFrame>, k_max_frames_in_flight> render_frames{};
			uint32_t active_frame_index{ 0u };
			bool frame_active{ false };

			edge::gfx::ISemaphore* acquired_semaphore{ nullptr };
			edge::gfx::ISemaphore* previous_frame_semaphore{ nullptr };
			edge::gfx::ICommandList* current_command_list{ nullptr };

			std::vector<std::unique_ptr<ExecutionNode>> graph_nodes{};
			std::vector<RenderResource> render_resources{};

			edge::gfx::ISampler* default_sampler{ nullptr };

			// Performance counters
			edge::gfx::IQueryPool* timestamp_query{ nullptr };
			float timestamp_frequency{ 0.f };
			float gpu_delta_time{ 0.f };

			// Shared descriptors
			edge::gfx::ISharedDescriptor* shared_descriptor{ nullptr };
			edge::gfx::IResourceDescriptor* global_resource_descriptor{ nullptr };
			std::unique_ptr<UniformPool> global_uniform_pool{};

			ResourceContainer<TextureObject> static_textures{};
			ResourceContainer<TextureObject> render_targets{};
			ResourceContainer<BufferObject> buffers{};
		};
	}
}