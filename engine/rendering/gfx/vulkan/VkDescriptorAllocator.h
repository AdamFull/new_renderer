#pragma once

#include <gfx_base.h>

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			class Device;
			class DescriptorPool;
			class DescriptorSetLayout;
			class DescriptorSet;
			class VulkanResourceDescriptor;

			class VulkanDescriptorAllocator final : public DescriptorAllocator
			{
			public:
				VulkanDescriptorAllocator(Device& device, uint32_t grow_factor = 2u);
				VulkanDescriptorAllocator(const VulkanDescriptorAllocator&) = delete;
				VulkanDescriptorAllocator(VulkanDescriptorAllocator&& other);
				~VulkanDescriptorAllocator();

				VulkanDescriptorAllocator& operator=(const VulkanDescriptorAllocator&) = delete;
				VulkanDescriptorAllocator& operator=(VulkanDescriptorAllocator&&) = delete;

				void SetResourceName(const char* name) override;

				void Reset() override;
				ResourceDescriptor* AllocateDescriptors(RootSignature* root_signature) override;
			private:
				DescriptorSet& Allocate(DescriptorSetLayout& layout);
			private:
				Device& device;

				// TODO: Do i even need to allocate memory here?
				uint32_t multiplicator{ 2u };
				std::vector<std::unique_ptr<DescriptorPool>> descriptor_pools{};
				std::vector<VulkanResourceDescriptor*> allocated_descriptors{};
			};
		}
	}
}