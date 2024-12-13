#include "UniformPool.h"

#include <edge/gfx/gfx_context.h>

#include <memory.hpp>

#include <tracy/Tracy.hpp>

namespace engine
{
	namespace render
	{
		constexpr int32_t k_max_frames_idle{ 300 };

		UniformPoolFrame::UniformPoolFrame(edge::gfx::Context& context, uint64_t whole_size) :
			context{ context },
			expired_countdown{ k_max_frames_idle }
		{
			edge::gfx::BufferCreateInfo create_info{};
			create_info.usage = edge::gfx::BufferUsageFlag::eUniformBuffer;
			create_info.size = whole_size;

			buffer = context.CreateBuffer(create_info);
			size_remain = buffer->GetSize();
		}

		UniformPoolFrame::UniformPoolFrame(UniformPoolFrame&& other) :
			context{ context },
			buffer{ std::exchange(other.buffer, {}) },
			size_remain{ std::exchange(other.size_remain, {}) },
			expired_countdown{ std::exchange(other.expired_countdown, {}) }
		{

		}

		UniformPoolFrame::~UniformPoolFrame()
		{
			context.Destroy(buffer);
		}

		bool UniformPoolFrame::IsExpired() const
		{
			return expired_countdown < 0;
		}

		void UniformPoolFrame::Reset() const
		{
			size_remain = buffer->GetSize();
			--expired_countdown;
		}

		bool UniformPoolFrame::HasEnoughtSpace(uint64_t requested_size) const
		{
			return static_cast<int64_t>(size_remain) - static_cast<int64_t>(requested_size) > 0;
		}

		edge::gfx::BufferSuballocation UniformPoolFrame::RequestBlock(uint64_t requested_size)
		{
			edge::gfx::BufferSuballocation suballocation{ *buffer, buffer->GetSize() - size_remain, requested_size };
			expired_countdown = k_max_frames_idle;
			size_remain -= requested_size;
			return suballocation;
		}


		UniformPool::UniformPool(edge::gfx::Context& context, uint64_t frame_size) :
			context{ context },
			frame_size{ frame_size }
		{
			context.RequestMinimalBufferAlignment(edge::gfx::BufferUsageFlag::eUniformBuffer, minimal_alignment);

			pool_frames.emplace_back(std::make_unique<UniformPoolFrame>(context, frame_size));
		}

		UniformPool::UniformPool(UniformPool&& other) :
			context{ other.context },
			frame_size{ std::exchange(other.frame_size, {}) },
			pool_frames{ std::exchange(other.pool_frames, {}) }
		{
		}

		void UniformPool::Reset()
		{
			ZoneScoped;

			auto to_delete = std::remove_if(pool_frames.begin(), pool_frames.end(), [](const std::unique_ptr<UniformPoolFrame>& frame)
				{
					frame->Reset();
					return frame->IsExpired();
				});

			if (to_delete != pool_frames.end())
			{
				pool_frames.erase(to_delete);
			}
		}

		edge::gfx::BufferSuballocation UniformPool::RequestBlock(uint64_t requested_size)
		{
			uint64_t aligned_size = utl::aligned_size(requested_size, minimal_alignment);
			auto found = std::find_if(pool_frames.begin(), pool_frames.end(), [&aligned_size](const std::unique_ptr<UniformPoolFrame>& frame)
				{
					return frame->HasEnoughtSpace(aligned_size);
				});
			;
			if (found != pool_frames.end())
			{
				return (*found)->RequestBlock(aligned_size);
			}

			auto& new_pool = pool_frames.emplace_back(std::make_unique<UniformPoolFrame>(context, frame_size));
			return new_pool->RequestBlock(aligned_size);
		}
	}
}