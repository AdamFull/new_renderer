#pragma once

#include "VkDescriptorSet.h"
#include <Base.h>

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			class DescriptorPool;
			class VulkanResourceDescriptor;

			class VulkanSharedDescriptor final : public SharedDescriptor
			{
			public:
				VulkanSharedDescriptor(Device& device, const SharedDescriptorCreateInfo& create_info);
				~VulkanSharedDescriptor();

				VulkanSharedDescriptor(const VulkanSharedDescriptor&) = delete;

				VulkanSharedDescriptor(VulkanSharedDescriptor&& other);

				VulkanSharedDescriptor& operator=(const VulkanSharedDescriptor&) = delete;
				VulkanSharedDescriptor& operator=(VulkanSharedDescriptor&&) = delete;

				DescriptorSetLayout& GetDescriptorSetLayout();

				//DescriptorSet& GetDescriptorSet();

				uint32_t GetFirstSet() const;

				void Reset() override;
				ResourceDescriptor* RequestDescriptors() override;
			private:
				Device& device;

				uint32_t first_set{};

				std::unique_ptr<DescriptorSetLayout> descriptor_set_layout{};
				std::unique_ptr<DescriptorPool> descriptor_pool{};

				std::vector<VulkanResourceDescriptor*> allocated_resource_descriptors{};
			};
		}
	}
}