#include <engine/render/render_api/vulkan/RenderContext.h>
#include <engine/render/render_api/vulkan/PhysicalDevice.h>
#include <engine/render/render_api/vulkan/Device.h>
#include <engine/render/render_api/vulkan/Swapchain.h>
#include <engine/render/render_api/vulkan/CommandPool.h>
#include <engine/render/render_api/vulkan/VkRenderDevice.h>

#include <engine/runtime/Window.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			RenderContext::RenderContext(RenderDevice& render_device, Window* window) :
				render_device{ render_device },
				window{ *window },
				queue{ render_device.GetLogicalDevice().GetQueueByPresent(0u) }
			{
#ifdef VK_USE_PLATFORM_ANDROID_KHR
				vk::PresentModeKHR present_mode = (window->GetProperties().vsync == Window::Vsync::OFF) ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
				std::vector<vk::PresentModeKHR> present_mode_priority_list{ vk::PresentModeKHR::eFifo, vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eImmediate };
#else
				vk::PresentModeKHR present_mode = (window->GetProperties().vsync == Window::Vsync::ON) ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eMailbox;
				std::vector<vk::PresentModeKHR> present_mode_priority_list{ vk::PresentModeKHR::eMailbox, vk::PresentModeKHR::eFifo, vk::PresentModeKHR::eImmediate };
#endif
				swapchain = std::make_unique<Swapchain>(render_device, 
					render_device.GetSurface(), 
					present_mode, 
					present_mode_priority_list, 
					render_device.GetSurfacePriorityList()
				);

				auto image_count = swapchain->GetImageCount();
				const auto& extent = swapchain->GetExtent();

				for (int32_t i = 0; i < image_count; ++i)
				{
					auto& render_frame = render_frames.emplace_back(std::make_unique<RenderFrame>(render_device, queue));
					render_frame->UpdateBackbuffer(extent, swapchain->GetFormat(), swapchain->GetImage(i));
				}
			}

			RenderContext::~RenderContext()
			{
				render_frames.clear();
				swapchain.reset();
			}

			void RenderContext::UpdateSwapchain(const vk::Extent2D& extent)
			{
				swapchain = std::make_unique<Swapchain>(*swapchain, extent);
				UpdateResources();
			}

			void RenderContext::UpdateSwapchain(const vk::Extent2D& extent, const vk::SurfaceTransformFlagBitsKHR transform)
			{
				vk::Extent2D new_extent{ extent };

				if (transform == vk::SurfaceTransformFlagBitsKHR::eRotate90 || transform == vk::SurfaceTransformFlagBitsKHR::eRotate270)
				{
					// Pre-rotation: always use native orientation i.e. if rotated, use width and height of identity transform
					std::swap(new_extent.width, new_extent.height);
				}

				swapchain = std::make_unique<Swapchain>(*swapchain, new_extent, transform);
				pre_transform = transform;

				UpdateResources();
			}

			void RenderContext::UpdateSwapchain(uint32_t image_count)
			{
				render_device.Wait();
				swapchain = std::make_unique<Swapchain>(*swapchain, image_count);
				UpdateResources();
			}

			bool RenderContext::HandleSurfaceChange(bool force_update)
			{
				if (!swapchain)
				{
					log_warning("Can't handle surface changes in headless mode, skipping.");
					return false;
				}

				auto& gpu = render_device.GetPhysicalDevice();
				vk::SurfaceCapabilitiesKHR surface_properties = Handle(gpu).getSurfaceCapabilitiesKHR(swapchain->GetSurface());
				if (surface_properties.currentExtent.width == 0xFFFFFFFFu)
				{
					return false;
				}

				auto& current_extent = swapchain->GetExtent();

				// Only recreate the swapchain if the dimensions have changed;
				// handle_surface_changes() is called on VK_SUBOPTIMAL_KHR,
				// which might not be due to a surface resize
				if (surface_properties.currentExtent.width != current_extent.width ||
					surface_properties.currentExtent.height != current_extent.height ||
					force_update)
				{
					// Recreate swapchain
					render_device.Wait();
					UpdateSwapchain(surface_properties.currentExtent, pre_transform);

					active_frame_index = 0u;
					current_frame_index = 0u;

					return true;
				}

				return false;
			}

			CommandBuffer& RenderContext::Begin(bool& surface_updated)
			{
				if (!frame_active)
				{
					BeginFrame(surface_updated);
				}

				if (!acquired_semaphore)
				{
					throw std::runtime_error("Couldn't begin frame");
				}

				auto& current_frame = GetCurrentFrame();
				CommandBuffer& command_buffer = current_frame.RequestCommandBuffer(vk::CommandBufferLevel::ePrimary);
				command_buffer.Begin(vk::CommandBufferUsageFlagBits::eRenderPassContinue);
				current_command_buffer = &command_buffer;
				return command_buffer;
			}

			void RenderContext::BeginFrame(bool& surface_updated)
			{
				if (swapchain)
				{
					surface_updated = HandleSurfaceChange();
				}

				assert(!frame_active && "Frame is still active, please call end_frame");

				auto& current_frame = GetCurrentFrame();

				// We will use the acquired semaphore in a different frame context,
				// so we need to hold ownership.
				acquired_semaphore = current_frame.GetImageAvaliableSemaphore();

				if (swapchain)
				{
					vk::Result result;
					try
					{
						result = swapchain->AcquireNextImage(acquired_semaphore, nullptr, active_frame_index);
					}
					catch (vk::OutOfDateKHRError& /*err*/)
					{
						result = vk::Result::eErrorOutOfDateKHR;
					}

					if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR)
					{
						surface_updated = HandleSurfaceChange(result == vk::Result::eErrorOutOfDateKHR);
						if (surface_updated)
						{
							result = swapchain->AcquireNextImage(acquired_semaphore, nullptr, active_frame_index);
						}
					}

					// eSuboptimalKHR can legitimately occur in batch mode when exiting certain samples (e.g. afbc, msaa, surface_rotation, swapchain_images)
					if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
					{
						current_frame.Reset();
						return;
					}
				}

				// Now the frame is active again
				frame_active = true;
				current_frame.Reset();
			}

			void RenderContext::EndFrame(vk::Semaphore wait_semaphore)
			{
				current_command_buffer->End();
				frame_active = false;

				auto& current_frame = GetCurrentFrame();
				current_frame.ResetFence();

				try
				{
					// TODO: COMMAND BUFFERS
					vk::SubmitInfo2 submit_info{};

					vk::SemaphoreSubmitInfo wait_semaphores[2] = { 
						vk::SemaphoreSubmitInfo{ acquired_semaphore, 0ull, vk::PipelineStageFlagBits2::eColorAttachmentOutput },
						vk::SemaphoreSubmitInfo{ wait_semaphore, 0ull, vk::PipelineStageFlagBits2::eAllCommands }
					};

					submit_info.pWaitSemaphoreInfos = &wait_semaphores[0];
					submit_info.waitSemaphoreInfoCount = wait_semaphore ? 2 : 1;

					vk::SemaphoreSubmitInfo signal_semaphore{};
					signal_semaphore.semaphore = current_frame.GetRenderFinishedSemaphore();
					signal_semaphore.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;

					submit_info.pSignalSemaphoreInfos = &signal_semaphore;
					submit_info.signalSemaphoreInfoCount = 1;

					vk::CommandBufferSubmitInfo command_buffer_si{};
					command_buffer_si.commandBuffer = Handle(*current_command_buffer);

					submit_info.pCommandBufferInfos = &command_buffer_si;
					submit_info.commandBufferInfoCount = 1;

					queue.Submit(submit_info, current_frame.GetFence());
				}
				catch (vk::SystemError error)
				{
					log_error("failed to submit Draw command buffer!");
				}

				if (swapchain)
				{
					vk::PresentInfoKHR present_info = {};
					present_info.waitSemaphoreCount = 1;
					present_info.pWaitSemaphores = &current_frame.GetRenderFinishedSemaphore();

					vk::SwapchainKHR swap_chains[] = { Handle(*swapchain) };
					present_info.swapchainCount = 1;
					present_info.pSwapchains = swap_chains;
					present_info.pImageIndices = &GetCurrentFrameIndex();

					auto& device = render_device.GetLogicalDevice();
					auto& surface_extent = swapchain->GetExtent();

					vk::DisplayPresentInfoKHR disp_present_info;
					if (device.IsExtensionSupported(VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME) &&
						window.GetDisplayPresentInfo(reinterpret_cast<VkDisplayPresentInfoKHR*>(&disp_present_info), surface_extent.width, surface_extent.height))
					{
						// Add display present info if supported and wanted
						present_info.pNext = &disp_present_info;
					}

					queue.Present(present_info);
					current_frame.WaitFence();

					current_frame_index = (current_frame_index + 1) % swapchain->GetImageCount();
				}

				previous_frame_semaphore = current_frame.GetRenderFinishedSemaphore();
				if (acquired_semaphore)
				{
					acquired_semaphore = nullptr;
				}
			}

			vk::Semaphore RenderContext::GetLastAcquiredSemaphore()
			{
				return previous_frame_semaphore;
			}

			CommandBuffer& RenderContext::GetCurrentCommandBuffer()
			{
				return *current_command_buffer;
			}

			RenderFrame& RenderContext::GetCurrentFrame()
			{
				return *render_frames[GetCurrentFrameIndex()];
			}

			const uint32_t& RenderContext::GetCurrentFrameIndex() const
			{
				return current_frame_index;
			}

			RenderDevice& RenderContext::GetDevice() const
			{
				return render_device;
			}

			Swapchain& RenderContext::GetSwapchain() const
			{
				return *swapchain;
			}

			void RenderContext::UpdateResources()
			{
				auto image_count = swapchain->GetImageCount();
				const auto& extent = swapchain->GetExtent();

				auto frame_it = render_frames.begin();
				for (int32_t i = 0; i < image_count; ++i)
				{
					if (frame_it != render_frames.end())
					{
						(*frame_it)->UpdateBackbuffer(extent, swapchain->GetFormat(), swapchain->GetImage(i));
					}
					else
					{
						// Create a new frame if the new swapchain has more images than current frames
						auto& render_frame = render_frames.emplace_back(std::make_unique<RenderFrame>(render_device, queue));
						render_frame->UpdateBackbuffer(extent, swapchain->GetFormat(), swapchain->GetImage(i));
					}

					++frame_it;
				}
			}
		}
	}
}