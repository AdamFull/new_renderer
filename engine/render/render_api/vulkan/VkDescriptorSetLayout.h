#pragma once

#include <VkResource.h>

namespace gfx
{
	namespace vulkan
	{
		class Device;

		static constexpr uint32_t k_max_pool_sizes_count{ 11u };
		static constexpr uint32_t k_max_layout_bindings{ 16u };

		using PoolSizes = std::array<int32_t, k_max_pool_sizes_count>;

		struct DescriptorSetLayoutCreateInfo
		{
			DescriptorSetLayoutCreateInfo();

			vk::DescriptorSetLayoutCreateInfo create_info{};
			vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT ext_create_info{};

			std::array<vk::DescriptorSetLayoutBinding, k_max_layout_bindings> bindings{};
			std::array<vk::DescriptorBindingFlagsEXT, k_max_layout_bindings> binding_flags_ext{};
		};

		class DescriptorSetLayout final : public VulkanResource<vk::DescriptorSetLayout>
		{
		public:
			struct DescriptorBindingInfo
			{
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

			const LayoutBindingArray& GetBindings() const;
			uint32_t GetBindingCount() const;

			vk::DescriptorSetLayoutBinding& GetLayoutBinding(uint32_t binding_index);
			vk::DescriptorBindingFlagsEXT& GetLayoutBindingFlags(uint32_t binding_index);

			const PoolSizes& GetRequiredDescriptorCount() const;
		private:
			Device& device;

			PoolSizes pool_sizes{};
			LayoutBindingArray layout_bindings{};
			uint32_t binding_count{ 0u };
		};
	}
}