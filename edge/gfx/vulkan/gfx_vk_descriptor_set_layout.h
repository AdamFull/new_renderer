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

	static constexpr int32_t k_max_pool_sizes_count{ 11 };

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

		auto valid() const noexcept -> bool override;
		auto allocated() const noexcept -> bool override {
			return descriptor_set || !invalidated_sets.empty();
		}

		auto hash() const noexcept -> uint32_t override {
			return layout_hash;
		}

		auto bind(const SamplerDescriptor& sampler_desc) -> void override;
		auto bind(const TextureDescriptor& texture_desc) -> void override;
		auto bind(const BufferDescriptor& buffer_desc) -> void override;

		auto update() -> void;

		auto set_descriptor_set(vk::DescriptorSet new_set, int32_t pool_idx) -> void;

		auto get_descriptor_set() const noexcept -> std::tuple<vk::DescriptorSet, int32_t> {
			return { descriptor_set, descriptor_pool_idx };
		}

		auto get_invalidated_descriptor_sets() const noexcept -> std::span<const std::pair<vk::DescriptorSet, int32_t>> {
			return invalidated_sets;
		}

		auto get_binding_count() const noexcept -> uint32_t {
			return binding_count;
		}

		auto get_required_descriptor_count() const noexcept -> PoolSizes const& {
			return pool_sizes;
		}
	private:
		Device& device;

		vk::DescriptorSet descriptor_set{ nullptr };
		std::vector<std::pair<vk::DescriptorSet, int32_t>> invalidated_sets{};
		int32_t descriptor_pool_idx{ -1 };

		PoolSizes pool_sizes{};
		uint32_t binding_count{ 0u };
		uint32_t layout_hash{ 0u };
		bool have_bindless_descriptor{ false };
		bool is_invalidated{ false };

		std::vector<vk::WriteDescriptorSet> write_descriptor_sets{};
		std::deque<vk::DescriptorImageInfo> image_descriptors{};
		std::deque<vk::DescriptorBufferInfo> buffer_descriptors{};

		mutable std::string descriptor_name{};
	};
}