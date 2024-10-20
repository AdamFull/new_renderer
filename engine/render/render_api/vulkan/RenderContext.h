#pragma once

#include <RenderFrame.h>

namespace engine
{
	class Window;

	namespace render
	{
		namespace vulkan
		{
			class Swapchain;
			class Queue;
			class RenderDevice;
			class CommandBuffer;

			class RenderContext final
			{
			public:
				RenderContext(RenderDevice& render_device, Window* window);
				~RenderContext();

				RenderContext(const RenderContext&) = delete;
				RenderContext(RenderContext&&) = delete;
				RenderContext& operator=(const RenderContext&) = delete;
				RenderContext& operator=(RenderContext&&) = delete;

				void CreateGlobalDescriptors(const FGlobalUniformCreateInfo& uniform_ci);

				void UpdateSwapchain(const vk::Extent2D& extent);
				void UpdateSwapchain(const vk::Extent2D& extent, const vk::SurfaceTransformFlagBitsKHR transform);
				void UpdateSwapchain(uint32_t image_count);

				bool HandleSurfaceChange(bool force_update = false);

				CommandBuffer& Begin(bool& surface_updated);

				void BeginFrame(bool& surface_updated);
				void EndFrame(vk::SemaphoreSubmitInfo wait_semaphore);

				vk::SemaphoreSubmitInfo GetUploadAvaliableSemaphore();

				CommandBuffer& GetCurrentCommandBuffer();
				RenderFrame& GetCurrentFrame();

				const uint32_t& GetCurrentFrameIndex() const;

				RenderDevice& GetDevice() const;
				Swapchain& GetSwapchain() const;

				DescriptorSetLayout& GetGlobalDescriptorSetLayout() const;
				DescriptorSet& GetCurrentGlobalDescriptorSet() const;
				Buffer* GetGlobalUniformBuffer(uint64_t uniform_name) const;
			protected:
				void UpdateResources();
			private:
				RenderDevice& render_device;

				const Window& window;
				const Queue& queue;

				std::unique_ptr<Swapchain> swapchain;
				vk::SurfaceTransformFlagBitsKHR pre_transform{ vk::SurfaceTransformFlagBitsKHR::eIdentity };

				static constexpr size_t k_max_frames_in_flight{ 8ull };
				std::array<std::unique_ptr<RenderFrame>, k_max_frames_in_flight> render_frames{};
				uint32_t active_frame_index{ 0u };
				uint32_t current_frame_index{ 0u };
				bool frame_active{ false };

				vk::Semaphore acquired_semaphore{ nullptr };
				vk::Semaphore previous_frame_semaphore{ nullptr };
				CommandBuffer* current_command_buffer{ nullptr };

				// Global descriptor
				std::unique_ptr<DescriptorAllocator> descriptor_allocator{};
				std::unique_ptr<DescriptorSetLayout> descriptor_set_layout{};
				std::array<std::unique_ptr<DescriptorSet>, k_max_descriptor_sets> descriptor_sets{};
				utl::hash_map<u64, std::unique_ptr<Buffer>> uniform_buffers{};
			};
		}
	}
}