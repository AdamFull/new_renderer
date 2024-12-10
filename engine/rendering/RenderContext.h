#pragma once

#include "RenderResourceContainer.h"

#include "ExecutionContext.h"
#include <span>

#include "gfx/gfx_base.h"

namespace engine
{
	class Window;

	namespace gfx
	{
		class Context;
		class ResourceLoader;
	}

	namespace render
	{
		class IRenderer;
		class UniformPool;

		class RenderContext final : public ExecutionContext
		{
		public:
			RenderContext(IRenderer& renderer, Window* window);
			~RenderContext();

			RenderContext(const RenderContext&) = delete;
			RenderContext(RenderContext&&) = delete;
			RenderContext& operator=(const RenderContext&) = delete;
			RenderContext& operator=(RenderContext&&) = delete;

			uint32_t AddResource() override;
			uint32_t AddResource(std::string_view path, gfx::Sampler* sampler = nullptr) override;
			uint32_t AddResource(const TextureResourceCreateInfo& create_info, std::span<uint8_t> texture_data) override;
			uint32_t AddResource(const TextureResourceCreateInfo& create_info, gfx::Sampler* sampler = nullptr) override;
			uint32_t AddResource(const gfx::BufferCreateInfo& create_info) override;

			void SetResourceName(uint32_t resource_index, std::string_view name) override;

			RenderResource& GetResource(uint32_t resource_index) override;
			RenderResource const& GetResource(uint32_t resource_index) const override;

			std::unique_ptr<ExecutionNode> CreateNode(std::string_view group_name, uint32_t group_color = 0xFFFFFFFF) override;
			std::unique_ptr<ExecutionNode> CreateNode(const RenderShaderPassCreateInfo& create_info) override;
			std::unique_ptr<ExecutionNode> CreateNode(const ComputeShaderPassCreateInfo& create_info) override;

			void AddNode(std::unique_ptr<ExecutionNode>&& node) override;
			void Execute(float delta_time) override;

			void UpdateGlobalBinding(uint32_t binding, std::span<const uint8_t> data);

			void Begin() override;
			void End() override;

			void UpdateSwapchain(uint32_t width, uint32_t height);
			void UpdateSwapchain(uint32_t image_count);

			bool HandleSurfaceChange(bool force_update = false);

			gfx::CommandList& BeginCommands(bool& surface_updated);

			void BeginFrame(bool& surface_updated);
			void EndFrame(gfx::SemaphoreSubmitInfo wait_semaphore);

			gfx::SemaphoreSubmitInfo GetUploadAvaliableSemaphore();

			gfx::Context& GetContext() override;
			gfx::Context const& GetContext() const override;

			gfx::Texture* GetTexture(uint32_t resource_index) override;
			gfx::Texture* GetTexture(const RenderResource& render_resource) override;

			gfx::Sampler* GetSampler(uint32_t resource_index) override;
			gfx::Sampler* GetSampler(const RenderResource& render_resource) override;

			gfx::Buffer* GetBuffer(uint32_t resource_index) override;
			gfx::Buffer* GetBuffer(const RenderResource& render_resource) override;

			gfx::BufferSuballocation RequestUniformBlock(uint64_t block_size) override;
			gfx::CommandList& GetCurrentCommandList() override;
			gfx::DescriptorAllocator& GetCurrentDescriptorAllocator() override;
		protected:
			const uint32_t& GetCurrentFrameIndex() const;

			class RenderFrame;
			RenderFrame& GetCurrentFrame();
			void UpdateRenderTargets();
			void UpdateResources();
		private:
			gfx::Context& context;
			gfx::ResourceLoader& resource_loader;

			const Window& window;
			const gfx::Queue* queue;

			gfx::Swapchain* swapchain{ nullptr };
			//vk::SurfaceTransformFlagBitsKHR pre_transform{ vk::SurfaceTransformFlagBitsKHR::eIdentity };

			static constexpr size_t k_max_frames_in_flight{ 8ull };
			std::array<std::unique_ptr<RenderFrame>, k_max_frames_in_flight> render_frames{};
			uint32_t active_frame_index{ 0u };
			uint32_t current_frame_index{ 0u };
			bool frame_active{ false };

			gfx::Semaphore* acquired_semaphore{ nullptr };
			gfx::Semaphore* previous_frame_semaphore{ nullptr };
			gfx::CommandList* current_command_list{ nullptr };

			std::vector<std::unique_ptr<ExecutionNode>> graph_nodes{};
			std::vector<RenderResource> render_resources{};

			gfx::Sampler* default_sampler{ nullptr };

			// Performance counters
			gfx::QueryPool* timestamp_query{ nullptr };
			float timestamp_frequency{ 0.f };
			float gpu_delta_time{ 0.f };

			// Shared descriptors
			gfx::SharedDescriptor* shared_descriptor{ nullptr };
			gfx::ResourceDescriptor* global_resource_descriptor{ nullptr };
			std::unique_ptr<UniformPool> global_uniform_pool{};

			ResourceContainer<TextureObject> static_textures{};
			ResourceContainer<TextureObject> render_targets{};
			ResourceContainer<BufferObject> buffers{};
		};
	}
}