#pragma once

#include "VkResource.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class Device;

			static constexpr uint32_t k_max_pool_sizes_count{ 11u };
			static constexpr uint32_t k_max_layout_bindings{ 16u };

			using PoolSizes = std::array<int32_t, k_max_pool_sizes_count>;

			struct DescriptorSetLayoutCreateInfo {
				DescriptorSetLayoutCreateInfo();

				vk::DescriptorSetLayoutCreateInfo create_info{};
				vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT ext_create_info{};

				std::array<vk::DescriptorSetLayoutBinding, k_max_layout_bindings> bindings{};
				std::array<vk::DescriptorBindingFlagsEXT, k_max_layout_bindings> binding_flags_ext{};

				vk::DescriptorSetLayoutBinding& add_binding();
				vk::DescriptorBindingFlagsEXT& add_binding_flags();
			};

			class DescriptorSetLayout final : public VulkanResource<vk::DescriptorSetLayout> {
			public:
				struct DescriptorBindingInfo {
					vk::DescriptorSetLayoutBinding binding{};
					vk::DescriptorBindingFlagsEXT flags{};
				};

				using LayoutBindingArray = std::array<DescriptorBindingInfo, k_max_layout_bindings>;

				DescriptorSetLayout(Device& device, const DescriptorSetLayoutCreateInfo& create_info);
				DescriptorSetLayout(const DescriptorSetLayout&) = delete;
				DescriptorSetLayout(DescriptorSetLayout&& other);
				~DescriptorSetLayout();

				DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
				DescriptorSetLayout& operator=(DescriptorSetLayout&&) = delete;

				auto get_bindings() const noexcept -> LayoutBindingArray const& {
					return layout_bindings;
				}

				auto get_binding_count() const noexcept -> uint32_t {
					return binding_count;
				}

				auto get_layout_binding(uint32_t binding_index) const noexcept -> vk::DescriptorSetLayoutBinding const& {
					return layout_bindings[binding_index].binding;
				}

				auto get_layout_binding_flags(uint32_t binding_index) const noexcept -> vk::DescriptorBindingFlagsEXT const& {
					return layout_bindings[binding_index].flags;
				}

				auto get_required_descriptor_count() const noexcept -> PoolSizes const& {
					return pool_sizes;
				}
			private:
				Device& device;

				PoolSizes pool_sizes{};
				LayoutBindingArray layout_bindings{};
				uint32_t binding_count{ 0u };
			};
		}
	}
}