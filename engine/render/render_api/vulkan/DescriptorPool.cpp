#include <engine/render/render_api/vulkan/DescriptorPool.h>
#include <engine/render/render_api/vulkan/VkRenderDevice.h>

// 1. 

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			PoolSizes default_pool_sizes
			{
				1u, // vk::DescriptorType::eSampler
				4u, // vk::DescriptorType::eCombinedImageSampler
				4u, // vk::DescriptorType::eSampledImage
				1u, // vk::DescriptorType::eStorageImage
				1u, // vk::DescriptorType::eUniformTexelBuffer
				1u, // vk::DescriptorType::eStorageTexelBuffer
				2u, // vk::DescriptorType::eUniformBuffer
				2u, // vk::DescriptorType::eStorageBuffer
				1u, // vk::DescriptorType::eUniformBufferDynamic
				1u, // vk::DescriptorType::eStorageBufferDynamic
				1u  // vk::DescriptorType::eInputAttachment
			};

			PoolSizes MergePoolSizes(const PoolSizes& lhs, const PoolSizes& rhs, uint32_t mult = 1u)
			{
				PoolSizes new_sizes{ lhs };

				for (int32_t i = 0; i < k_max_pool_sizes_count; ++i)
					new_sizes[i] = (new_sizes[i] + rhs[i]) * mult;

				return new_sizes;
			}

			bool HaveEnoughtSizes(const PoolSizes& lhs, const PoolSizes& rhs)
			{
				for (int32_t i = 0; i < k_max_pool_sizes_count; ++i)
				{
					if (lhs[i] - rhs[i] < 0)
						return false;
				}

				return true;
			}

			DescriptorPool::DescriptorPool(RenderDevice& render_device) :
				render_device{ render_device }
			{
				
			}

			DescriptorPool::DescriptorPool(DescriptorPool&& other) :
				render_device{ other.render_device }
			{

			}

			DescriptorPool::~DescriptorPool()
			{
				for (auto& pool : descriptor_pools)
				{
					render_device.Destroy(pool.handle);
				}
			}

			void DescriptorPool::Reset()
			{
				for (auto& pool : descriptor_pools)
				{
					render_device.ResetDescriptorPool2(pool.handle);
					pool.allocated_count = 0ull;

					// Reset avaliable descriptors
					std::copy(pool.initial_sizes.begin(), pool.initial_sizes.end(), pool.avaliable_sizes.begin());
				}
			}

			std::unique_ptr<DescriptorSet> DescriptorPool::Allocate(DescriptorSetLayout& layout, const std::string& debug_name)
			{
				// We didn't have any pool for now, needs to allocate new one
				if (descriptor_pools.empty())
				{
					AllocateNewDescriptorPool(layout);
				}

				const auto& required_descriptor_count = layout.GetRequiredDescriptorCount();
				auto found = std::find_if(descriptor_pools.begin(), descriptor_pools.end(), [&required_descriptor_count](const DescriptorPoolBlock& pool)
					{
						return pool.allocated_count < DescriptorPoolBlock::k_max_sets && HaveEnoughtSizes(pool.avaliable_sizes, required_descriptor_count);
					});

				DescriptorPoolBlock* pool{ nullptr };
				if (found != descriptor_pools.end())
				{
					pool = &(*found);
				}
				else
				{
					AllocateNewDescriptorPool(layout);
					pool = &descriptor_pools.back();
				}

				assert(pool);

				vk::DescriptorSetAllocateInfo alloc_info{};
				alloc_info.descriptorPool = pool->handle;
				alloc_info.descriptorSetCount = 1;
				alloc_info.pSetLayouts = &Handle(layout);

				vk::DescriptorSet descriptor_set{ VK_NULL_HANDLE };
				render_device.AllocateDescriptorSet2(alloc_info, &descriptor_set, debug_name);
				assert(descriptor_set);

				for (int32_t i = 0; i < k_max_pool_sizes_count; ++i)
				{
					pool->avaliable_sizes[i] -= required_descriptor_count[i];
				}
				++pool->allocated_count;

				return std::make_unique<DescriptorSet>(*this, descriptor_set, pool - &descriptor_pools[0], required_descriptor_count);
			}

			vk::Result DescriptorPool::Free(DescriptorSet& descriptor_set)
			{
				if (descriptor_set.handle)
				{
					auto& pool = descriptor_pools[descriptor_set.pool_index];
					render_device.FreeDescriptorSets2(pool.handle, &descriptor_set.handle, 1u);
					descriptor_set.handle = nullptr;

					--pool.allocated_count;

					for (int32_t i = 0; i < k_max_pool_sizes_count; ++i)
					{
						pool.avaliable_sizes[i] += descriptor_set.used_sizes[i];
					}
				}

				return vk::Result::eSuccess;
			}

			void DescriptorPool::AllocateNewDescriptorPool(DescriptorSetLayout& layout)
			{
				auto& requested_sizes = layout.GetRequiredDescriptorCount();
				bool have_previous_pools = !descriptor_pools.empty();

				auto& new_pool = descriptor_pools.emplace_back();
				if (have_previous_pools)
				{
					auto& last_pool = descriptor_pools[descriptor_pools.size() - 2ull];

					// If in last pool allocated_count is reached limit and sizes was enought, we save the size
					if (HaveEnoughtSizes(last_pool.initial_sizes, requested_sizes) && last_pool.allocated_count > DescriptorPoolBlock::k_max_sets)
					{
						// We have reached max sets limit. We can save previous pool sizes
						std::copy(last_pool.initial_sizes.begin(), last_pool.initial_sizes.end(), new_pool.initial_sizes.begin());
					}
					else
					{
						new_pool.initial_sizes = MergePoolSizes(last_pool.initial_sizes, requested_sizes, multiplicator);
					}
				}
				else
				{
					new_pool.initial_sizes = MergePoolSizes(default_pool_sizes, requested_sizes, multiplicator);
				}

				// Reset avaliable sizes
				std::copy(new_pool.initial_sizes.begin(), new_pool.initial_sizes.end(), new_pool.avaliable_sizes.begin());

				// Prepare pool sizes
				std::array<vk::DescriptorPoolSize, k_max_pool_sizes_count> pool_sizes{};
				for (int32_t i = 0; i < k_max_pool_sizes_count; ++i)
				{
					pool_sizes[i] = { static_cast<vk::DescriptorType>(i), static_cast<uint32_t>(new_pool.initial_sizes[i]) };
				}

				vk::DescriptorPoolCreateInfo create_info{};
				create_info.poolSizeCount = k_max_pool_sizes_count;
				create_info.pPoolSizes = pool_sizes.data();
				create_info.maxSets = DescriptorPoolBlock::k_max_sets;

				for (int32_t i = 0; i < layout.GetBindingCount(); ++i)
				{
					auto& flags = layout.GetLayoutBindingFlags(i);
					if (flags & vk::DescriptorBindingFlagBitsEXT::eUpdateAfterBind)
					{
						create_info.flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT;
						break;
					}
				}

				new_pool.handle = render_device.CreateDescriptorPool2(create_info);
			}
		}
	}
}