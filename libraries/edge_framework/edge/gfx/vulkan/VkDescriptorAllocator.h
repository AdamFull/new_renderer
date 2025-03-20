#pragma once

#include "../gfx_base.h"

namespace edge
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

			class VulkanDescriptorAllocator final : public IDescriptorAllocator {
			public:
				VulkanDescriptorAllocator(Device& device, uint32_t grow_factor = 2u);
				VulkanDescriptorAllocator(const VulkanDescriptorAllocator&) = delete;
				VulkanDescriptorAllocator(VulkanDescriptorAllocator&& other);
				~VulkanDescriptorAllocator();

				VulkanDescriptorAllocator& operator=(const VulkanDescriptorAllocator&) = delete;
				VulkanDescriptorAllocator& operator=(VulkanDescriptorAllocator&&) = delete;

				void set_resource_name(std::string_view name) const noexcept override;

				void reset() override;
				auto allocate_descriptors(IRootSignature* root_signature) -> IResourceDescriptor* override;
			private:
				auto allocate(DescriptorSetLayout& layout) -> DescriptorSet&;
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