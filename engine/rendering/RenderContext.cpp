#include "RenderContext.h"
#include "UniformPool.h"

#include <engine/runtime/Window.h>

#include "gfx/gfx_context.h"
#include "gfx/gfx_util.h"

#include <format>

#include <logger.hpp>

#include <tracy/Tracy.hpp>

namespace engine
{
	namespace render
	{
		RenderContext::RenderContext(gfx::Context& context, Window* window) :
			context{ context },
			window{ *window }
		{
			// Select queue that can present
			queue = context.RequestQueue(gfx::QueueTypeFlag::ePresent);
			
			gfx::SwapChainCreateInfo swapchain_ci{};
			swapchain_ci.window = window;
			swapchain_ci.image_count = 3u;

			swapchain = context.CreateSwapchain(swapchain_ci);

			auto image_count = swapchain->GetImageCount();

			for (int32_t i = 0; i < image_count; ++i)
			{
				render_frames[i] = CreateRenderFrame(i);
			}
		}

		RenderContext::~RenderContext()
		{
			queue->WaitIdle();

			auto image_count = swapchain->GetImageCount();
			for (int32_t i = 0; i < image_count; ++i)
			{
				DestroyRenderFrame(render_frames[i]);
			}

			context.Destroy(swapchain);
		}

		//void RenderContext::CreateGlobalDescriptors(const FGlobalUniformCreateInfo& uniform_ci)
		//{
		//
		//}

		void RenderContext::UpdateSwapchain(uint32_t width, uint32_t height)
		{
			gfx::SwapChainCreateInfo new_swapchain_ci{};
			new_swapchain_ci.window = &window;
			new_swapchain_ci.old_swapchain = swapchain;
			new_swapchain_ci.width = width;
			new_swapchain_ci.height = height;

			swapchain = context.CreateSwapchain(new_swapchain_ci);
			context.Destroy(new_swapchain_ci.old_swapchain);
			UpdateResources();
		}

		void RenderContext::UpdateSwapchain(uint32_t image_count)
		{
			gfx::SwapChainCreateInfo new_swapchain_ci{};
			new_swapchain_ci.window = &window;
			new_swapchain_ci.old_swapchain = swapchain;
			new_swapchain_ci.image_count = image_count;

			swapchain = context.CreateSwapchain(new_swapchain_ci);
			context.Destroy(new_swapchain_ci.old_swapchain);
			UpdateResources();
		}

		bool RenderContext::HandleSurfaceChange(bool force_update)
		{
			ZoneScoped;

			if (!swapchain)
			{
				log_warning("Can't handle surface changes in headless mode, skipping.");
				return false;
			}

			uint32_t surface_width, surface_height;
			swapchain->GetSurfaceExtent(surface_width, surface_height);
			if (surface_width == 0xFFFFFFFFu)
			{
				return false;
			}

			auto& current_width = swapchain->GetWidth();
			auto& current_height = swapchain->GetHeight();

			// Only recreate the swapchain if the dimensions have changed;
			// handle_surface_changes() is called on VK_SUBOPTIMAL_KHR,
			// which might not be due to a surface resize
			if (surface_width != current_width ||
				surface_height != current_height ||
				force_update)
			{
				ZoneScopedN("Swapchain update");
				// Recreate swapchain
				queue->WaitIdle();
				UpdateSwapchain(surface_width, surface_height);

				active_frame_index = 0u;
				current_frame_index = 0u;

				return true;
			}

			return false;
		}

		gfx::CommandList& RenderContext::Begin(bool& surface_updated)
		{
			ZoneScoped;

			if (!frame_active)
			{
				BeginFrame(surface_updated);
			}

			if (!acquired_semaphore)
			{
				throw std::runtime_error("Couldn't begin frame");
			}

			auto& current_frame = GetCurrentFrame();
			current_command_list = current_frame.command_pool->Allocate();
			current_command_list->Begin();
			return *current_command_list;
		}

		void RenderContext::BeginFrame(bool& surface_updated)
		{
			ZoneScoped;

			if (swapchain)
			{
				surface_updated = HandleSurfaceChange();
			}

			assert(!frame_active && "Frame is still active, please call end_frame");

			auto& current_frame = GetCurrentFrame();

			// We will use the acquired semaphore in a different frame context,
			// so we need to hold ownership.
			acquired_semaphore = current_frame.image_avaliable;

			if (swapchain)
			{
				if (!swapchain->AcquireNextImage(acquired_semaphore, nullptr, &active_frame_index))
				{
					surface_updated = HandleSurfaceChange(true);

					bool acquire_result{ false };
					if (surface_updated)
					{
						acquire_result = swapchain->AcquireNextImage(acquired_semaphore, nullptr, &active_frame_index);
					}

					if (!acquire_result)
					{
						current_frame.command_pool->Reset();
						current_frame.uniform_pool->Reset();
						return;
					}
				}
			}

			// Now the frame is active again
			frame_active = true;
			current_frame.descriptor_allocator->Reset();
			current_frame.command_pool->Reset();
			current_frame.uniform_pool->Reset();
		}

		void RenderContext::EndFrame(gfx::SemaphoreSubmitInfo wait_semaphore)
		{
			ZoneScoped;

			auto& current_frame = GetCurrentFrame();

			current_command_list->End();
			frame_active = false;

			current_frame.fence->Reset();

			try
			{
				ZoneScopedN("Submit");

				// TODO: COMMAND BUFFERS
				gfx::SubmitInfo submit_info{};

				gfx::SemaphoreSubmitInfo wait_semaphores[2] = {
					gfx::SemaphoreSubmitInfo{ acquired_semaphore, 0ull, gfx::StageFlag::eColorAttachmentOutput },
					wait_semaphore
				};

				submit_info.wait_semaphore_infos = &wait_semaphores[0];
				submit_info.wait_semaphore_count = wait_semaphore.semaphore ? 2 : 1;


				gfx::SemaphoreSubmitInfo signal_semaphores[2] = {
					gfx::SemaphoreSubmitInfo{ current_frame.render_finished, 0ull, gfx::StageFlag::eColorAttachmentOutput },
					gfx::SemaphoreSubmitInfo{ current_frame.upload_avaliable, 0ull, gfx::StageFlag::eAllCommands }
				};

				submit_info.signal_semaphore_infos = &signal_semaphores[0];
				submit_info.signal_semaphore_count = 1u;

				submit_info.command_lists = &current_command_list;
				submit_info.command_buffer_count = 1u;

				queue->Submit(submit_info, current_frame.fence);
			}
			catch (std::exception error)
			{
				log_error("failed to submit Draw command buffer!");
			}

			if (swapchain)
			{
				ZoneScopedN("Present");

				auto* wait_semaphore = current_frame.render_finished;

				gfx::PresentInfo present_info = {};
				present_info.wait_semaphore_count = 1;
				present_info.wait_semaphores = &wait_semaphore;

				present_info.swapchain_count = 1;
				present_info.swapchains = &swapchain;
				present_info.image_indices = &GetCurrentFrameIndex();

				queue->Present(present_info);
				current_frame.fence->Wait();

				current_frame_index = (current_frame_index + 1) % swapchain->GetImageCount();
			}

			previous_frame_semaphore = current_frame.upload_avaliable;
			if (acquired_semaphore)
			{
				acquired_semaphore = nullptr;
			}
		}

		gfx::SemaphoreSubmitInfo RenderContext::GetUploadAvaliableSemaphore()
		{
			return gfx::SemaphoreSubmitInfo{ previous_frame_semaphore, 0ull, gfx::StageFlag::eAllCommands };
		}

		gfx::BufferSuballocation RenderContext::RequestUniformBlock(uint64_t block_size)
		{
			auto& current_frame = GetCurrentFrame();
			return current_frame.uniform_pool->RequestBlock(block_size);
		}

		gfx::CommandList& RenderContext::GetCurrentCommandList()
		{
			return *current_command_list;
		}

		gfx::DescriptorAllocator& RenderContext::GetCurrentDescriptorAllocator()
		{
			auto& current_frame = GetCurrentFrame();
			return *current_frame.descriptor_allocator;
		}

		gfx::Texture& RenderContext::GetBackbuffer()
		{
			return *GetCurrentFrame().backbuffer;
		}

		const uint32_t& RenderContext::GetCurrentFrameIndex() const
		{
			return current_frame_index;
		}

		gfx::Swapchain& RenderContext::GetSwapchain() const
		{
			return *swapchain;
		}

		//DescriptorSetLayout& GetGlobalDescriptorSetLayout() const;
		//gfx::DescriptorSet& GetCurrentGlobalDescriptorSet() const;

		bool RenderContext::CanUpdateUniform() const
		{
			return true;
		}

		bool RenderContext::CanUpdateTextureBinding() const
		{
			return true;
		}

		void RenderContext::UpdateUniform(uint32_t binding, std::span<const char> data)
		{

		}

		void RenderContext::UpdateTextureBinding(gfx::Texture& texture, gfx::Sampler& sampler, uint32_t binding_slot)
		{

		}

		void RenderContext::UpdateDescriptors()
		{

		}

		RenderFrame RenderContext::CreateRenderFrame(int32_t frame_index)
		{
			RenderFrame render_frame{};

			render_frame.backbuffer = context.CreateSwapchainTexture(swapchain, frame_index);
			render_frame.command_pool = context.CreateCommandPool(queue);
			render_frame.descriptor_allocator = context.CreateDescriptorAllocator();
			render_frame.uniform_pool = std::make_unique<UniformPool>(context, 16384ull);
			render_frame.image_avaliable = context.CreateSemaphore();
			render_frame.image_avaliable->SetResourceName(std::format("image_avaliable_{}", frame_index).c_str());
			render_frame.render_finished = context.CreateSemaphore();
			render_frame.render_finished->SetResourceName(std::format("render_finished_{}", frame_index).c_str());
			render_frame.upload_avaliable = context.CreateSemaphore();
			render_frame.render_finished->SetResourceName(std::format("upload_avaliable_{}", frame_index).c_str());
			render_frame.fence = context.CreateFence();

			return render_frame;
		}

		void RenderContext::DestroyRenderFrame(RenderFrame& render_frame)
		{
			context.Destroy(render_frame.backbuffer);
			context.Destroy(render_frame.command_pool);
			context.Destroy(render_frame.descriptor_allocator);
			context.Destroy(render_frame.image_avaliable);
			context.Destroy(render_frame.render_finished);
			context.Destroy(render_frame.upload_avaliable);
			context.Destroy(render_frame.fence);
			render_frame.uniform_pool.reset();
		}

		RenderFrame& RenderContext::GetCurrentFrame()
		{
			return render_frames[GetCurrentFrameIndex()];
		}
	
		void RenderContext::UpdateResources()
		{
			ZoneScoped;

			auto image_count = swapchain->GetImageCount();

			auto frame_it = render_frames.begin();
			for (int32_t i = 0; i < image_count; ++i)
			{
				if (frame_it != render_frames.end())
				{
					context.Destroy(frame_it->backbuffer);
					frame_it->backbuffer = context.CreateSwapchainTexture(swapchain, i);
				}
				else
				{
					// Create a new frame if the new swapchain has more images than current frames
					render_frames[i] = CreateRenderFrame(i);
				}

				++frame_it;
			}
		}
	}
}