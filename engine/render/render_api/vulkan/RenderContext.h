#pragma once

#include <vulkan/vulkan.hpp>

namespace engine
{
	class Window;

	namespace render
	{
		namespace vulkan
		{
			class Device;
			class Swapchain;
			class RenderDevice;

			class RenderContext
			{
			public:
				RenderContext(Window* window, RenderDevice& render_device);
				~RenderContext();

				int32_t GetCurrentFrameIndex() const;

				Device& GetDevice() const;
				Swapchain& GetSwapchain() const;
			private:
				Device& device;

				std::unique_ptr<Swapchain> swapchain;

				std::vector<vk::Semaphore> image_avaliable_semaphores{};
				std::vector<vk::Semaphore> render_finished_semaphores{};
				std::vector<vk::Fence> in_flight_fences{};

				int32_t frame_index;
			};
		}
	}
}