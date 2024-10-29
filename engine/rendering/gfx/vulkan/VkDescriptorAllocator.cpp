#include "VkDescriptorAllocator.h"
#include "VkDevice.h"
#include "VkDescriptorPool.h"
#include "VkRootSignature.h"

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			VulkanDescriptorAllocator::VulkanDescriptorAllocator(Device& device, uint32_t grow_factor) :
				device{ device },
				multiplicator{ grow_factor }
			{
			}

			VulkanDescriptorAllocator::VulkanDescriptorAllocator(VulkanDescriptorAllocator&& other) :
				device{ other.device },
				descriptor_pools{ std::exchange(other.descriptor_pools, {}) }
			{
			}

			VulkanDescriptorAllocator::~VulkanDescriptorAllocator()
			{
				descriptor_pools.clear();
			}

			void VulkanDescriptorAllocator::Reset()
			{
				auto to_delete = std::remove_if(descriptor_pools.begin(), descriptor_pools.end(), [](const std::unique_ptr<DescriptorPool>& pool)
					{
						pool->Reset();
						return pool->IsExpired();
					});

				if (to_delete != descriptor_pools.end())
				{
					descriptor_pools.erase(to_delete);
				}
			}

			void VulkanDescriptorAllocator::AllocateSets(RootSignature* root_signature)
			{
				auto* vk_root_signature = static_cast<VulkanRootSignature*>(root_signature);

				auto& set_layouts = vk_root_signature->GetDescriptorSetLayouts();
				for (int32_t i = 0; i < k_max_descriptor_count; ++i)
				{
					// TODO: check is uses external set layout (ignore externals)

					auto& layout = set_layouts[i];
					if (layout)
					{
						// Update sets
						vk_root_signature->descriptor_sets[i] = Allocate(*layout);
					}
				}
			}

			DescriptorSet* VulkanDescriptorAllocator::Allocate(DescriptorSetLayout& layout)
			{
				// We didn't have any pool for now, needs to allocate new one
				if (descriptor_pools.empty())
				{
					auto& new_pool = descriptor_pools.emplace_back(std::make_unique<DescriptorPool>(device, layout));
					return new_pool->Allocate(layout);
				}

				const auto& required_descriptor_count = layout.GetRequiredDescriptorCount();
				auto found = std::find_if(descriptor_pools.begin(), descriptor_pools.end(), [&layout](const std::unique_ptr<DescriptorPool>& pool)
					{
						return pool->CanAllocate(layout) == CanAllocateResult::Success;
					});
				;
				if (found != descriptor_pools.end())
				{
					return (*found)->Allocate(layout);
				}

				auto& new_pool = descriptor_pools.emplace_back(std::make_unique<DescriptorPool>(device, layout, !descriptor_pools.empty() ? descriptor_pools.back().get() : nullptr));
				return new_pool->Allocate(layout);
			}
		}
	}
}