#include "VkDescriptorSetLayout.h"
#include "VkDevice.h"

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			DescriptorSetLayoutCreateInfo::DescriptorSetLayoutCreateInfo()
			{
				ext_create_info.pBindingFlags = binding_flags_ext.data();

				create_info.pBindings = bindings.data();
				create_info.pNext = &ext_create_info;
			}

			vk::DescriptorSetLayoutBinding& DescriptorSetLayoutCreateInfo::AddBinding()
			{
				return bindings[create_info.bindingCount++];
			}

			vk::DescriptorBindingFlagsEXT& DescriptorSetLayoutCreateInfo::AddBindingFlags()
			{
				return binding_flags_ext[ext_create_info.bindingCount++];
			}

			DescriptorSetLayout::DescriptorSetLayout(Device& device, const DescriptorSetLayoutCreateInfo& create_info) :
				VulkanResource{ nullptr, device.GetAllocationCallbacks() },
				device{ device }
			{
				vk::Result result = Handle(device).createDescriptorSetLayout(&create_info.create_info, allocation_callbacks, &handle);

				for (int32_t i = 0; i < create_info.create_info.bindingCount; ++i)
				{
					auto& binding = create_info.bindings[i];

					// Count number of required descriptors
					pool_sizes[utl::enum_cast(binding.descriptorType)] += binding.descriptorCount;

					layout_bindings[i] = { binding, create_info.binding_flags_ext[i] };
					++binding_count;
				}
			}

			DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) :
				VulkanResource{ std::move(other) },
				device{ other.device },
				layout_bindings{ std::exchange(other.layout_bindings, {}) },
				binding_count{ std::exchange(other.binding_count, {}) },
				pool_sizes{ std::exchange(other.pool_sizes, {}) }
			{
			}

			DescriptorSetLayout::~DescriptorSetLayout()
			{
				Handle(device).destroyDescriptorSetLayout(handle, allocation_callbacks);
			}

			const DescriptorSetLayout::LayoutBindingArray& DescriptorSetLayout::GetBindings() const
			{
				return layout_bindings;
			}

			uint32_t DescriptorSetLayout::GetBindingCount() const
			{
				return binding_count;
			}

			vk::DescriptorSetLayoutBinding& DescriptorSetLayout::GetLayoutBinding(uint32_t binding_index)
			{
				return layout_bindings[binding_index].binding;
			}

			vk::DescriptorBindingFlagsEXT& DescriptorSetLayout::GetLayoutBindingFlags(uint32_t binding_index)
			{
				return layout_bindings[binding_index].flags;
			}

			const PoolSizes& DescriptorSetLayout::GetRequiredDescriptorCount() const
			{
				return pool_sizes;
			}
		}
	}
}