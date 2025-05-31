#pragma once

#include "../gfx_base.h"
#include "gfx_vk_resource.h"

#include <algorithm>
#include <numeric>
#include <compare>
#include <concepts>

namespace edge::gfx::vulkan {

	template<typename _Ty>
	concept arithmetic = std::is_arithmetic_v<_Ty>;

	class Device;

	static constexpr int32_t k_max_pool_sizes_count{ 12 };

	struct PoolSizes {
		std::array<int32_t, k_max_pool_sizes_count> sizes{};

		// Arithmetic operators (compound-assignment)
		PoolSizes& operator+=(const PoolSizes& rhs) {
			std::transform(sizes.begin(), sizes.end(), rhs.sizes.begin(), sizes.begin(), std::plus{});
			return *this;
		}

		PoolSizes& operator-=(const PoolSizes& rhs) {
			std::transform(sizes.begin(), sizes.end(), rhs.sizes.begin(), sizes.begin(), std::minus{});
			return *this;
		}

		PoolSizes& operator*=(const PoolSizes& rhs) {
			std::transform(sizes.begin(), sizes.end(), rhs.sizes.begin(), sizes.begin(), std::multiplies{});
			return *this;
		}

		PoolSizes& operator/=(const PoolSizes& rhs) {
			std::transform(sizes.begin(), sizes.end(), rhs.sizes.begin(), sizes.begin(), std::divides{});
			return *this;
		}

		// Scalar arithmetic operators (compound-assignment)
		template<arithmetic _Ty>
		PoolSizes& operator+=(_Ty rhs) {
			for (auto& s : sizes) s += rhs;
			return *this;
		}

		template<arithmetic _Ty>
		PoolSizes& operator-=(_Ty rhs) {
			for (auto& s : sizes) s -= rhs;
			return *this;
		}

		template<arithmetic _Ty>
		PoolSizes& operator*=(_Ty rhs) {
			for (auto& s : sizes) s *= rhs;
			return *this;
		}

		template<arithmetic _Ty>
		PoolSizes& operator/=(_Ty rhs) {
			for (auto& s : sizes) s /= rhs;
			return *this;
		}

		// Non-member arithmetic operators
		friend PoolSizes operator+(PoolSizes lhs, const PoolSizes& rhs) { return lhs += rhs; }
		friend PoolSizes operator-(PoolSizes lhs, const PoolSizes& rhs) { return lhs -= rhs; }
		friend PoolSizes operator*(PoolSizes lhs, const PoolSizes& rhs) { return lhs *= rhs; }
		friend PoolSizes operator/(PoolSizes lhs, const PoolSizes& rhs) { return lhs /= rhs; }

		template<arithmetic _Ty>
		friend PoolSizes operator+(PoolSizes lhs, _Ty rhs) { return lhs += rhs; }
		template<arithmetic _Ty>
		friend PoolSizes operator-(PoolSizes lhs, _Ty rhs) { return lhs -= rhs; }
		template<arithmetic _Ty>
		friend PoolSizes operator*(PoolSizes lhs, _Ty rhs) { return lhs *= rhs; }
		template<arithmetic _Ty>
		friend PoolSizes operator/(PoolSizes lhs, _Ty rhs) { return lhs /= rhs; }

		//inline friend bool operator==(const PoolSizes& lhs, const PoolSizes& rhs) {
		//	for (int32_t i = 0; i < k_max_pool_sizes_count; ++i) {
		//		if (lhs.sizes[i] != rhs.sizes[i])
		//			return false;
		//	}
		//
		//	return true;
		//}

		//inline friend bool operator!=(const PoolSizes& lhs, const PoolSizes& rhs) { return !(lhs == rhs); }
		//
		//inline friend bool operator<(const PoolSizes& lhs, const PoolSizes& rhs)  {
		//	for (int32_t i = 0; i < k_max_pool_sizes_count; ++i) {
		//		if (lhs.sizes[i] - rhs.sizes[i] < 0)
		//			return true;
		//	}
		//
		//	return false;
		//}
		//
		//inline friend bool operator>(const PoolSizes& lhs, const PoolSizes& rhs) { return rhs < lhs; }
		//inline friend bool operator<=(const PoolSizes& lhs, const PoolSizes& rhs) { return !(lhs > rhs); }
		//inline friend bool operator>=(const PoolSizes& lhs, const PoolSizes& rhs) { return !(lhs < rhs); }

		bool operator==(const PoolSizes&) const = default;

		auto operator<=>(const PoolSizes& rhs) const {
			return sizes <=> rhs.sizes;
		}

		int32_t& operator[](size_t idx) { return sizes[idx]; }
		const int32_t& operator[](size_t idx) const { return sizes[idx]; }
	};

	class DescriptorSetLayout final : public IDescriptorLayout, public Resource<vk::DescriptorSetLayout> {
	public:
		DescriptorSetLayout(Device& device, const DescriptorLayoutCreateInfo& create_info);
		DescriptorSetLayout(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout(DescriptorSetLayout&& other);
		~DescriptorSetLayout();

		DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout& operator=(DescriptorSetLayout&&) = delete;

		void set_resource_name(std::string_view name) const noexcept override;

		auto is_bindless() const noexcept -> bool;

		auto get_binding_count() const noexcept -> uint32_t {
			return binding_count;
		}

		auto get_required_descriptor_count() const noexcept -> PoolSizes const& {
			return pool_sizes;
		}
	private:
		Device& device;

		PoolSizes pool_sizes{};
		uint32_t binding_count{ 0u };
		bool have_bindless_descriptor{ false };
	};
}