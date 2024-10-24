#pragma once

#include <engine/render/ExecutionContext.h>
#include <engine/render/RenderFrame.h>
#include <span>

#include <GFXStructures.h>

namespace engine
{
	class Window;

	namespace gfx
	{
		class Swapchain;
		class Sampler;
	}

	namespace render
	{
		class RenderContext final : ExecutionContext
		{
		public:
			RenderContext(gfx::Context& context, Window* window);
			~RenderContext();

			RenderContext(const RenderContext&) = delete;
			RenderContext(RenderContext&&) = delete;
			RenderContext& operator=(const RenderContext&) = delete;
			RenderContext& operator=(RenderContext&&) = delete;

			//void CreateGlobalDescriptors(const FGlobalUniformCreateInfo& uniform_ci);

			void UpdateSwapchain(uint32_t width, uint32_t height);
			void UpdateSwapchain(uint32_t image_count);

			bool HandleSurfaceChange(bool force_update = false);

			gfx::CommandList& Begin(bool& surface_updated);

			void BeginFrame(bool& surface_updated);
			void EndFrame(gfx::SemaphoreSubmitInfo wait_semaphore);

			gfx::SemaphoreSubmitInfo GetUploadAvaliableSemaphore();

			gfx::CommandList& GetCurrentCommandList();
			RenderFrame& GetCurrentFrame();

			const uint32_t& GetCurrentFrameIndex() const;

			gfx::Swapchain& GetSwapchain() const;

			//DescriptorSetLayout& GetGlobalDescriptorSetLayout() const;
			//gfx::DescriptorSet& GetCurrentGlobalDescriptorSet() const;

			bool CanUpdateUniform() const;
			bool CanUpdateTextureBinding() const;

			void UpdateUniform(uint32_t binding, std::span<const char> data);
			void UpdateTextureBinding(gfx::Texture& texture, gfx::Sampler& sampler, uint32_t binding_slot);
			void UpdateDescriptors();
		protected:
			void UpdateResources();
		private:
			gfx::Context& context;

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

			// Global descriptor
			//std::unique_ptr<DescriptorAllocator> descriptor_allocator{};
			//std::unique_ptr<DescriptorSetLayout> descriptor_set_layout{};
			//std::vector<vk::DescriptorSet> descriptor_sets;
			//std::unique_ptr<UniformPool> uniform_pool{};
			//DescriptorUpdate descriptor_update{};
		};
	}
}