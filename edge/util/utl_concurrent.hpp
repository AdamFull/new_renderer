#pragma once

#include <atomic>
#include <type_traits>

namespace edge::utl {

	template<typename _Ty, size_t _N>
	class spsc_ring_buffer {
	public:
		spsc_ring_buffer() = default;
		~spsc_ring_buffer() = default;

		spsc_ring_buffer(const spsc_ring_buffer&) = delete;
		spsc_ring_buffer& operator=(const spsc_ring_buffer&) = delete;
		spsc_ring_buffer(spsc_ring_buffer&&) = delete;
		spsc_ring_buffer& operator=(spsc_ring_buffer&&) = delete;

		template<typename U>
		bool try_push(U&& item) {
			const size_t current_write = write_pos_.load(std::memory_order_relaxed);
			const size_t next_write = (current_write + 1) & MASK;

			if (next_write == read_pos_.load(std::memory_order_acquire)) {
				return false;
			}

			buffer_[current_write] = std::forward<U>(item);

			write_pos_.store(next_write, std::memory_order_release);

			return true;
		}

		bool try_pop(_Ty& item) {
			const size_t current_read = read_pos_.load(std::memory_order_relaxed);

			if (current_read == write_pos_.load(std::memory_order_acquire)) {
				return false;
			}

			item = std::move(buffer_[current_read]);

			const size_t next_read = (current_read + 1) & MASK;
			read_pos_.store(next_read, std::memory_order_release);

			return true;
		}

		bool empty() const {
			return read_pos_.load(std::memory_order_relaxed) ==
				write_pos_.load(std::memory_order_relaxed);
		}

		size_t size() const {
			const size_t write = write_pos_.load(std::memory_order_relaxed);
			const size_t read = read_pos_.load(std::memory_order_relaxed);
			return (write - read) & MASK;
		}

		static constexpr size_t capacity() { return _N - 1; }
	private:
		static_assert((_N & (_N - 1)) == 0, "Buffer size must be power of 2");
		static constexpr size_t MASK = _N - 1;

		alignas(64) std::atomic<size_t> write_pos_{ 0 };
		alignas(64) std::atomic<size_t> read_pos_{ 0 };

		alignas(64) _Ty buffer_[_N];
	};
}