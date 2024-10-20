#include <RenderContext.h>
#include <base/PhysicalDevice.h>
#include <base/Device.h>
#include <base/Swapchain.h>
#include <base/CommandPool.h>
#include <VkRenderDevice.h>
#include <DescriptorAllocator.h>
#include <base/Buffer.h>
#include <UniformPool.h>
#include <Texture.h>
#include <base/ImageView.h>
#include <base/Sampler.h>

#include <engine/runtime/Window.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			static const u32 k_max_bindless_resources{ 16384u };
			static const u64 k_max_texture_update_per_frame{ 64ull };
			static const u64 k_max_buffer_update_per_frame{ 64ull };

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
					render_frames[i] = std::make_unique<RenderFrame>(render_device, queue);
					render_frames[i]->UpdateBackbuffer(extent, swapchain->GetFormat(), swapchain->GetImage(i));
				}
			}

			RenderContext::~RenderContext()
			{
				for (int32_t i = 0; i < swapchain->GetImageCount(); ++i)
				{
					render_frames[i].reset();
				}

				swapchain.reset();
			}

			void RenderContext::CreateGlobalDescriptors(const FGlobalUniformCreateInfo& uniform_ci)
			{
				// Create descriptor pool
				descriptor_allocator = std::make_unique<DescriptorAllocator>(render_device);

				// Create descriptor set layout
				auto descriptor_set_layout_builder = DescriptorSetLayoutBuilder();

				// Frame data
				vk::DescriptorSetLayoutBinding global_frame_data_binding{};
				global_frame_data_binding.descriptorType = vk::DescriptorType::eUniformBuffer;
				global_frame_data_binding.descriptorCount = 1u;
				global_frame_data_binding.binding = 0u;
				global_frame_data_binding.stageFlags = vk::ShaderStageFlagBits::eAllGraphics | vk::ShaderStageFlagBits::eCompute;
				descriptor_set_layout_builder.AddLayoutBinding(global_frame_data_binding, vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind);

				// Uniform buffer with adresses to another buffers
				vk::DescriptorSetLayoutBinding global_buffer_references_binding{};
				global_buffer_references_binding.descriptorType = vk::DescriptorType::eUniformBuffer;
				global_buffer_references_binding.descriptorCount = 1u;
				global_buffer_references_binding.binding = 1u;
				global_buffer_references_binding.stageFlags = vk::ShaderStageFlagBits::eAllGraphics | vk::ShaderStageFlagBits::eCompute;
				descriptor_set_layout_builder.AddLayoutBinding(global_buffer_references_binding, vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind);

				// Uniform buffer with post processing data
				vk::DescriptorSetLayoutBinding global_postprocess_binding{};
				global_postprocess_binding.descriptorType = vk::DescriptorType::eUniformBuffer;
				global_postprocess_binding.descriptorCount = 1u;
				global_postprocess_binding.binding = 2u;
				global_postprocess_binding.stageFlags = vk::ShaderStageFlagBits::eAllGraphics | vk::ShaderStageFlagBits::eCompute;
				descriptor_set_layout_builder.AddLayoutBinding(global_postprocess_binding, vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind);

				// Bindles textures
				vk::DescriptorSetLayoutBinding global_textures_binding{};
				global_textures_binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
				global_textures_binding.descriptorCount = k_max_bindless_resources;
				global_textures_binding.binding = 3u;
				global_textures_binding.stageFlags = vk::ShaderStageFlagBits::eAllGraphics | vk::ShaderStageFlagBits::eCompute;
				global_textures_binding.pImmutableSamplers = nullptr;
				descriptor_set_layout_builder.AddLayoutBinding(global_textures_binding, vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind);

				descriptor_set_layout_builder.SetDebugName("global");
				descriptor_set_layout = descriptor_set_layout_builder.BuildUnique(render_device);

				// TODO: i have to create new pool after recreating pipeline when count of frames was changed
				descriptor_sets.resize(swapchain->GetImageCount());
				for (auto& set : descriptor_sets)
				{
					set = descriptor_allocator->Allocate(*descriptor_set_layout).GetHandle();
				}

				uniform_pool = std::make_unique<UniformPool>(render_device, 4096ull);
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

			void RenderContext::EndFrame(vk::SemaphoreSubmitInfo wait_semaphore)
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
						wait_semaphore
					};

					submit_info.pWaitSemaphoreInfos = &wait_semaphores[0];
					submit_info.waitSemaphoreInfoCount = wait_semaphore.semaphore ? 2 : 1;


					vk::SemaphoreSubmitInfo signal_semaphores[2] = {
						vk::SemaphoreSubmitInfo{ current_frame.GetRenderFinishedSemaphore(), 0ull, vk::PipelineStageFlagBits2::eColorAttachmentOutput },
						vk::SemaphoreSubmitInfo{ current_frame.GetUploadAvaliableSemaphore(), 0ull, vk::PipelineStageFlagBits2::eAllCommands }
					};

					submit_info.pSignalSemaphoreInfos = &signal_semaphores[0];
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

				previous_frame_semaphore = current_frame.GetUploadAvaliableSemaphore();
				if (acquired_semaphore)
				{
					acquired_semaphore = nullptr;
				}
			}

			vk::SemaphoreSubmitInfo RenderContext::GetUploadAvaliableSemaphore()
			{
				return vk::SemaphoreSubmitInfo{ previous_frame_semaphore, 0ull, vk::PipelineStageFlagBits2::eAllCommands };
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

			DescriptorSetLayout& RenderContext::GetGlobalDescriptorSetLayout() const
			{
				return *descriptor_set_layout;
			}

			vk::DescriptorSet RenderContext::GetCurrentGlobalDescriptorSet() const
			{
				return descriptor_sets[GetCurrentFrameIndex()];
			}

			bool RenderContext::CanUpdateUniform() const
			{
				return descriptor_update.buffer_count < k_max_buffer_update_per_frame;
			}

			bool RenderContext::CanUpdateTextureBinding() const
			{
				return descriptor_update.image_count < k_max_texture_update_per_frame;
			}

			void RenderContext::UpdateUniform(uint32_t binding, std::span<const char> data)
			{
				auto requested_block = uniform_pool->RequestBlock(data.size());

				auto& buffer_descriptor = descriptor_update.AddBufferInfo();
				buffer_descriptor = requested_block.GetDescriptor();

				auto& write_descriptor_set = descriptor_update.AddDescriptorWrite();
				write_descriptor_set.descriptorType = vk::DescriptorType::eUniformBuffer;
				write_descriptor_set.dstSet = descriptor_sets[GetCurrentFrameIndex()];
				write_descriptor_set.dstBinding = binding;
				write_descriptor_set.descriptorCount = 1u;
				write_descriptor_set.pBufferInfo = &buffer_descriptor;

				requested_block.Update(data);
			}

			void RenderContext::UpdateTextureBinding(Texture& texture, Sampler& sampler, uint32_t binding_slot)
			{
				bool is_storage_image = utl::cast<bool>(texture.GetFlags() & ETextureCreateFlagBits::eStorage);

				auto& mip_level = texture.GetImageView();
				auto& image_descriptor = descriptor_update.AddImageInfo();
				image_descriptor.imageView = mip_level.GetHandle();
				image_descriptor.sampler = sampler.GetHandle();
				image_descriptor.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

				if (texture.GetFlags() & ETextureCreateFlagBits::eSamplable)
				{
					auto& write_descriptor_set = descriptor_update.AddDescriptorWrite();
					write_descriptor_set.dstArrayElement = binding_slot;
					write_descriptor_set.descriptorType = vk::DescriptorType::eCombinedImageSampler;
					write_descriptor_set.dstSet = descriptor_sets[GetCurrentFrameIndex()];
					write_descriptor_set.dstBinding = 3u;
					write_descriptor_set.descriptorCount = 1u;
					write_descriptor_set.pImageInfo = &image_descriptor;
				}
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
						render_frames[i] = std::make_unique<RenderFrame>(render_device, queue);
						render_frames[i]->UpdateBackbuffer(extent, swapchain->GetFormat(), swapchain->GetImage(i));
					}

					++frame_it;
				}
			}

			void RenderContext::UpdateDescriptors()
			{
				if (descriptor_update.writes_count)
				{
					auto& logical = render_device.GetLogicalDevice();
					Handle(logical).updateDescriptorSets(descriptor_update.writes_count, descriptor_update.write_descriptor_sets.data(), 0, nullptr);
					descriptor_update.Clear();
				}
			}
		}
	}
}