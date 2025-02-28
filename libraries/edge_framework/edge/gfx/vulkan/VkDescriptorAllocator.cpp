#include "VkDescriptorAllocator.h"
#include "VkDevice.h"
#include "VkDescriptorPool.h"
#include "VkRootSignature.h"
#include "VkResourceDescriptor.h"

#include "../../util/Profiler.h"

namespace edge
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

				for (auto& descriptor : allocated_descriptors)
				{
					delete descriptor;
					descriptor = nullptr;
				}
			}

			void VulkanDescriptorAllocator::SetResourceName(std::string_view name) const noexcept
			{
				assert(false && "NOT IMPLEMENTED");
			}

			void VulkanDescriptorAllocator::Reset()
			{
				EDGE_PROFILE_ZONE_SCOPED;

				auto to_delete = std::remove_if(descriptor_pools.begin(), descriptor_pools.end(), [](const std::unique_ptr<DescriptorPool>& pool)
					{
						pool->Reset();
						return pool->IsExpired();
					});

				if (to_delete != descriptor_pools.end())
				{
					descriptor_pools.erase(to_delete);
				}

				for (auto& descriptor : allocated_descriptors)
				{
					delete descriptor;
					descriptor = nullptr;
				}

				allocated_descriptors.clear();
			}

			auto VulkanDescriptorAllocator::AllocateDescriptors(IRootSignature* root_signature) -> IResourceDescriptor*
			{
				EDGE_PROFILE_ZONE_SCOPED;

				auto* vk_root_signature = static_cast<VulkanRootSignature*>(root_signature);

				auto& set_layouts = vk_root_signature->GetDescriptorSetLayouts();

				auto& new_resource_descriptor = allocated_descriptors.emplace_back(new VulkanResourceDescriptor(device));
				for (int32_t i = 0; i < k_max_descriptor_count; ++i)
				{
					// TODO: check is uses external set layout (ignore externals)

					auto& layout = set_layouts[i];
					if (layout)
					{
						auto& new_descriptor_set = Allocate(*layout);
						new_resource_descriptor->UpdateDescriptorSet(i, new_descriptor_set);
					}
				}

				return new_resource_descriptor;
			}

			auto VulkanDescriptorAllocator::Allocate(DescriptorSetLayout& layout) -> DescriptorSet&
			{
				EDGE_PROFILE_ZONE_SCOPED;

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