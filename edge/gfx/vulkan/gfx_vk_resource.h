#pragma once

#include <vulkan/vulkan.hpp>
#include "../../util/utl_hash.hpp"

namespace edge::gfx::vulkan
{
	template<typename _Ty>
	uint32_t handle_hash(const _Ty& handle, uint32_t seed = 0u)
	{
		auto handle_address = reinterpret_cast<uint64_t>(static_cast<_Ty::CType>(handle));
		return murmur_hash(handle_address, seed);
	}

	template<typename _Ty>
	class Resource {
	public:
		using HandleType = _Ty;

		explicit Resource(_Ty resource, vk::AllocationCallbacks* allocation_callbacks) :
			handle_{ resource },
			allocation_callbacks{ allocation_callbacks } {
		}

		Resource(const Resource&) = delete;
		Resource& operator=(const Resource&) = delete;

		Resource(Resource&& other) :
			handle_{ std::exchange(other.handle_, {}) },
			allocation_callbacks{ std::exchange(other.allocation_callbacks, {}) } {
		}

		Resource& operator=(Resource&& other) {
			handle_ = std::exchange(other.handle_, {});
			return *this;
		}

		virtual ~Resource() = default;

		_Ty& get_handle() {
			return handle_;
		}

		const _Ty& get_handle() const {
			return handle_;
		}

		vk::AllocationCallbacks* get_allocation_callbacks() const {
			return allocation_callbacks;
		}

		uint32_t hash() const {
			return handle_hash(handle_);
		}

		explicit operator bool() const noexcept {
			return handle_;
		}

		bool operator!() const noexcept {
			return !handle_;
		}
	protected:
		_Ty handle_;
		vk::AllocationCallbacks* allocation_callbacks{ nullptr };
	};
}