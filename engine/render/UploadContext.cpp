#include <engine/render/UploadContext.h>

#include <util/Logger.h>

#include <GFX.h>

namespace engine
{
	namespace render
	{
		UploadContext::UploadContext(gfx::Context& context) :
			context{ context }
		{
			queue = context.RequestQueue(gfx::QueueTypeFlag::eGraphics, 1u);

			assert(k_max_upload_frames > upload_frame_count);

			for (int32_t i = 0; i < upload_frame_count; ++i)
			{
				upload_frames[i] = std::make_unique<UploadFrame>(context, *queue);
			}
		}

		void UploadContext::BeginBufferUpdate(BufferUpdateInfo2& update_info)
		{
			BeginFrame();

			gfx::Buffer* dst_buffer = update_info.dst_buffer;
			assert(false && "NOT IMPLEMENTED");
			//if (dst_buffer->GetUsage() != vma::MemoryUsage::eGpuOnly)
			//{
			//	log_warning("This buffer can be updated from the CPU, no additional memory needs to be allocated.");
			//	update_info.map_info.data = dst_buffer->Map();
			//	update_info.map_info.avaliable_size = dst_buffer->GetSize();
			//}
			//else
			//{
			//	vk::DeviceSize allocation_size = update_info.update_size > 0ull ? update_info.update_size : dst_buffer->GetSize();
			//
			//	auto& upload_frame = GetActiveFrame();
			//	auto& src_buffer = upload_frame.RequestStagingBuffer(allocation_size, &update_info.map_info.buffer_index);
			//
			//	update_info.map_info.data = src_buffer.Map();
			//	update_info.map_info.avaliable_size = src_buffer.GetSize();
			//}
		}

		void UploadContext::EndBufferUpdate(BufferUpdateInfo2& update_info)
		{
			gfx::Buffer* dst_buffer = update_info.dst_buffer;

			if (update_info.map_info.avaliable_size == ~0ull)
			{
				dst_buffer->Unmap();
			}
			else
			{
				auto& upload_frame = GetActiveFrame();
				auto& command_buffer = upload_frame.AcquireOrGetCommandList();
				auto& src_buffer = upload_frame.GetStagingBuffer(update_info.map_info.buffer_index);

				// TODO: Add memory barrier befor copy for dst buffer
				gfx::BufferBarrier buffer_barrier{};
				buffer_barrier.buffer = dst_buffer;
				buffer_barrier.src_state.stage = gfx::StageFlag::eAllCommands;
				buffer_barrier.src_state.access =	gfx::AccessFlag::eTransferWrite | gfx::AccessFlag::eShaderWrite | gfx::AccessFlag::eMemoryWrite | gfx::AccessFlag::eShaderStorageWrite;
				buffer_barrier.dst_state.stage = gfx::StageFlag::eTransfer;
				buffer_barrier.dst_state.access = gfx::AccessFlag::eTransferWrite;

				gfx::ResourceBarrierInfo barrier_info{};
				barrier_info.buffer_barriers = &buffer_barrier;
				barrier_info.buffer_barrier_count = 1u;

				command_buffer.ResourceBarrier(barrier_info);

				update_info.map_info.buffer_index = ~0ull;
				assert(false && "NOT IMPLAMENTED");
				//src_buffer.CopyTo(Handle(command_buffer), *dst_buffer, update_info.copy_regions);
			}

			update_info.map_info.data = nullptr;
			update_info.map_info.avaliable_size = 0ull;
		}

		gfx::SemaphoreSubmitInfo UploadContext::Submit(gfx::SemaphoreSubmitInfo* wait_infos, uint32_t wait_semaphore_count)
		{
			if (writing_commands)
			{
				auto& upload_frame = GetActiveFrame();
				auto& command_buffer = upload_frame.AcquireOrGetCommandList();

				command_buffer.EndMarker();
				command_buffer.End();
				writing_commands = false;

				auto signal_semaphore_info = upload_frame.Submit(wait_infos, wait_semaphore_count);

				active_upload_frame = (active_upload_frame + 1u) % upload_frame_count;

				return signal_semaphore_info;
			}

			return {};
		}

		UploadFrame& UploadContext::GetActiveFrame()
		{
			return *upload_frames[active_upload_frame];
		}

		void UploadContext::BeginFrame()
		{
			if (!writing_commands)
			{
				auto& upload_frame = GetActiveFrame();
				upload_frame.Reset();

				auto& command_buffer = upload_frame.AcquireOrGetCommandList();

				command_buffer.Begin();
				command_buffer.BeginMarker("Upload resources", 0xFFFFFFFF);

				writing_commands = true;
			}
		}
	}
}