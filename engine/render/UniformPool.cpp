#include <engine/render/UniformPool.h>

#include <GFX.h>

namespace engine
{
	namespace render
	{
		constexpr int32_t k_max_frames_idle{ 300 };

		UniformPoolFrame::UniformPoolFrame(gfx::Context& context, uint64_t whole_size) :
			context{ context },
			expired_countdown{ k_max_frames_idle }
		{
			gfx::BufferCreateInfo create_info{};
			create_info.usage = gfx::BufferUsageFlag::eUniformBuffer;
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

		gfx::BufferSuballocation UniformPoolFrame::RequestBlock(uint64_t requested_size)
		{
			gfx::BufferSuballocation suballocation{ *buffer, buffer->GetSize() - size_remain, requested_size };
			expired_countdown = k_max_frames_idle;
			size_remain -= requested_size;
			return suballocation;
		}
	}
}