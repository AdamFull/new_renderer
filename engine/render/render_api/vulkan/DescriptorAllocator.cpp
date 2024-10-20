#include <DescriptorAllocator.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			DescriptorAllocator::DescriptorAllocator(RenderDevice& render_device) :
				render_device{ render_device }
			{
			}

			DescriptorAllocator::DescriptorAllocator(DescriptorAllocator&& other) :
				render_device{ other.render_device },
				descriptor_pools{ std::exchange(other.descriptor_pools, {}) }
			{
			}

			DescriptorAllocator::~DescriptorAllocator()
			{
				descriptor_pools.clear();
			}

			void DescriptorAllocator::Reset()
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

			DescriptorSet& DescriptorAllocator::Allocate(DescriptorSetLayout& layout, const std::string& debug_name)
			{
				// We didn't have any pool for now, needs to allocate new one
				if (descriptor_pools.empty())
				{
					auto& new_pool = descriptor_pools.emplace_back(std::make_unique<DescriptorPool>(render_device, layout));
					return new_pool->Allocate(layout, debug_name);
				}

				const auto& required_descriptor_count = layout.GetRequiredDescriptorCount();
				auto found = std::find_if(descriptor_pools.begin(), descriptor_pools.end(), [&layout](const std::unique_ptr<DescriptorPool>& pool)
					{
						return pool->CanAllocate(layout) == CanAllocateResult::Success;
					});
				;
				if (found != descriptor_pools.end())
				{
					return (*found)->Allocate(layout, debug_name);
				}

				auto& new_pool = descriptor_pools.emplace_back(std::make_unique<DescriptorPool>(render_device, layout, !descriptor_pools.empty() ? descriptor_pools.back().get() : nullptr));
				return new_pool->Allocate(layout, debug_name);
			}
		}
	}
}