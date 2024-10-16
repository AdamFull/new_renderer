#pragma once

#include <engine/render/render_api/vulkan/RenderFrame.h>

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

				void UpdateSwapchain(const vk::Extent2D& extent);
				void UpdateSwapchain(const vk::Extent2D& extent, const vk::SurfaceTransformFlagBitsKHR transform);
				void UpdateSwapchain(uint32_t image_count);

				bool HandleSurfaceChange(bool force_update = false);

				CommandBuffer& Begin(bool& surface_updated);

				void BeginFrame(bool& surface_updated);
				void EndFrame();

				CommandBuffer& GetCurrentCommandBuffer();
				RenderFrame& GetCurrentFrame();

				const uint32_t& GetCurrentFrameIndex() const;

				RenderDevice& GetDevice() const;
				Swapchain& GetSwapchain() const;
			protected:
				void UpdateResources();
			private:
				RenderDevice& render_device;

				const Window& window;
				const Queue& queue;

				std::unique_ptr<Swapchain> swapchain;
				vk::SurfaceTransformFlagBitsKHR pre_transform{ vk::SurfaceTransformFlagBitsKHR::eIdentity };

				std::vector<std::unique_ptr<RenderFrame>> render_frames{};
				uint32_t active_frame_index{ 0u };
				uint32_t current_frame_index{ 0u };
				bool frame_active{ false };

				vk::Semaphore acquired_semaphore{ nullptr };
				vk::Semaphore previous_frame_semaphore{ nullptr };
				CommandBuffer* current_command_buffer{ nullptr };
			};
		}
	}
}