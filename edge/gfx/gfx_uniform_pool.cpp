#include "gfx_uniform_pool.h"

#include "gfx_context.h"

#include <edge/util/utl_memory.hpp>

namespace edge::gfx {
	constexpr int32_t k_max_frames_idle{ 300 };

	class UniformPoolFrame final {
	public:
		UniformPoolFrame(uint64_t whole_size) :
			expired_countdown{ k_max_frames_idle } {
			BufferCreateInfo create_info{};
			create_info.usage = BufferUsageFlag::eUniformBuffer;
			create_info.size = whole_size;

			buffer = create_buffer_unique(create_info);
			size_remain = buffer->get_size();
		}

		UniformPoolFrame(const UniformPoolFrame&) = delete;
		UniformPoolFrame(UniformPoolFrame&& other) :
			buffer{ std::exchange(other.buffer, {}) },
			size_remain{ std::exchange(other.size_remain, {}) },
			expired_countdown{ std::exchange(other.expired_countdown, {}) } {

		}

		~UniformPoolFrame() = default;

		UniformPoolFrame& operator=(const UniformPoolFrame&) = delete;
		UniformPoolFrame& operator=(UniformPoolFrame&&) = delete;

		void unmap() {
			buffer->unmap();
		}

		auto is_expired() const -> bool {
			return expired_countdown < 0;
		}

		void reset() const {
			size_remain = buffer->get_size();
			--expired_countdown;
		}

		auto has_enought_space(uint64_t requested_size) const -> bool {
			return static_cast<int64_t>(size_remain) - static_cast<int64_t>(requested_size) > 0;
		}

		auto request_block(uint64_t requested_size) -> edge::gfx::BufferSuballocation {
			edge::gfx::BufferSuballocation suballocation{ *buffer, buffer->get_size() - size_remain, requested_size };
			expired_countdown = k_max_frames_idle;
			size_remain -= requested_size;
			return suballocation;
		}
	private:
		std::unique_ptr<IBuffer> buffer;
		mutable uint64_t size_remain{ 0ull };
		mutable int32_t expired_countdown{ 0 };
	};

	class UniformPool final : public IUniformPool {
	public:
		~UniformPool() = default;

		UniformPool(uint64_t frame_size) :
			frame_size{ frame_size } {
			edge::gfx::request_minimal_buffer_alignment(edge::gfx::BufferUsageFlag::eUniformBuffer, minimal_alignment);

			frames.emplace_back(std::make_unique<UniformPoolFrame>(frame_size));
		}

		UniformPool(UniformPool&& other) :
			frame_size{ std::exchange(other.frame_size, {}) },
			frames{ std::exchange(other.frames, {}) }
		{
		}

		void frame_mark() override {
			for (auto& frame : frames) {
				frame->unmap();
			}
		}

		void reset() override {
			auto to_delete = std::remove_if(frames.begin(), frames.end(),
				[](const std::unique_ptr<UniformPoolFrame>& frame) {
					frame->reset();
					return frame->is_expired();
				});

			if (to_delete != frames.end()) {
				frames.erase(to_delete, frames.end());
			}
		}

		auto request_block(uint64_t requested_size) -> edge::gfx::BufferSuballocation override {
			uint64_t aligned_size_ = aligned_size(requested_size, minimal_alignment);
			auto found = std::find_if(frames.begin(), frames.end(),
				[&aligned_size_](const std::unique_ptr<UniformPoolFrame>& frame) {
					return frame->has_enought_space(aligned_size_);
				});
			;
			if (found != frames.end()) {
				return (*found)->request_block(aligned_size_);
			}

			auto& new_pool = frames.emplace_back(std::make_unique<UniformPoolFrame>(frame_size));
			return new_pool->request_block(aligned_size_);
		}
	private:
		uint64_t minimal_alignment{};
		uint64_t frame_size{ 4096ull };
		std::vector<std::unique_ptr<UniformPoolFrame>> frames{};
	};

	auto create_uniform_pool(uint64_t frame_size) -> IUniformPool* {
		return new UniformPool(frame_size);
	}

	auto create_uniform_pool_unique(uint64_t frame_size) -> std::unique_ptr<IUniformPool> {
		return std::make_unique<UniformPool>(frame_size);
	}
}