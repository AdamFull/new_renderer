#include "VkDescriptorPool.h"
#include "VkDevice.h"

#include "../../util/Profiler.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			constexpr size_t k_max_sets{ 50ull };
			constexpr size_t k_max_frames_unused{ 300ull };

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

			DescriptorPool::DescriptorPool(Device& device, DescriptorSetLayout& layout, DescriptorPool* previous_pool) :
				VulkanResource{ nullptr, device.GetAllocationCallbacks() },
				device{ device }
			{
				auto& requested_sizes = layout.GetRequiredDescriptorCount();

				if (previous_pool)
				{
					switch (previous_pool->last_allocation_result)
					{
					case CanAllocateResult::MaxSetsLimit:
						std::copy(previous_pool->initial_sizes.begin(), previous_pool->initial_sizes.end(), initial_sizes.begin());
						break;
					case CanAllocateResult::MaxDescriptorLimit:
						initial_sizes = MergePoolSizes(previous_pool->initial_sizes, requested_sizes, 2);
						break;
					case CanAllocateResult::InvalidFlags: break;
					default: break;
					}
				}
				else
				{
					initial_sizes = MergePoolSizes(default_pool_sizes, requested_sizes, 2);
				}

				// Reset avaliable sizes
				std::copy(initial_sizes.begin(), initial_sizes.end(), avaliable_sizes.begin());

				// Prepare pool sizes
				std::array<vk::DescriptorPoolSize, k_max_pool_sizes_count> pool_sizes{};
				for (int32_t i = 0; i < k_max_pool_sizes_count; ++i)
				{
					pool_sizes[i] = { static_cast<vk::DescriptorType>(i), static_cast<uint32_t>(initial_sizes[i]) };
				}

				vk::DescriptorPoolCreateInfo create_info{};
				create_info.poolSizeCount = k_max_pool_sizes_count;
				create_info.pPoolSizes = pool_sizes.data();
				create_info.maxSets = k_max_sets;

				for (int32_t i = 0; i < layout.GetBindingCount(); ++i)
				{
					auto& flags = layout.GetLayoutBindingFlags(i);
					if (flags & vk::DescriptorBindingFlagBitsEXT::eUpdateAfterBind)
					{
						create_info.flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT;
						break;
					}
				}

				vk::Result result = Handle(device).createDescriptorPool(&create_info, allocation_callbacks, &handle);
				countdown_count = k_max_frames_unused;
			}

			DescriptorPool::DescriptorPool(DescriptorPool&& other) :
				VulkanResource{ std::move(other) },
				device{ other.device },
				initial_sizes{ std::exchange(other.initial_sizes, {}) },
				avaliable_sizes{ std::exchange(other.avaliable_sizes, {}) },
				allocated_count{ std::exchange(other.allocated_count, {}) },
				countdown_count{ std::exchange(other.countdown_count, {}) },
				allocated_descriptors{ std::exchange(other.allocated_descriptors, {}) }
			{
				allocated_descriptors.resize(k_max_sets);
			}

			DescriptorPool::~DescriptorPool()
			{
				Handle(device).destroyDescriptorPool(handle, allocation_callbacks);
			}

			void DescriptorPool::Reset()
			{
				EDGE_PROFILE_ZONE_SCOPED;

				Handle(device).resetDescriptorPool(handle);
				allocated_descriptors.clear();

				last_allocation_result = CanAllocateResult::Success;
				allocated_count = 0u;
				--countdown_count;

				// Reset avaliable descriptors
				std::copy(initial_sizes.begin(), initial_sizes.end(), avaliable_sizes.begin());
			}

			DescriptorSet& DescriptorPool::Allocate(DescriptorSetLayout& layout, std::string_view debug_name)
			{
				EDGE_PROFILE_ZONE_SCOPED;

				vk::DescriptorSetAllocateInfo alloc_info{};
				alloc_info.descriptorPool = handle;
				alloc_info.descriptorSetCount = 1;
				alloc_info.pSetLayouts = &layout.GetHandle();

				vk::DescriptorSet descriptor_set{ VK_NULL_HANDLE };
				vk::Result result = Handle(device).allocateDescriptorSets(&alloc_info, &descriptor_set);

				assert(descriptor_set);

				auto& required_descriptor_count = layout.GetRequiredDescriptorCount();
				for (int32_t i = 0; i < k_max_pool_sizes_count; ++i)
				{
					avaliable_sizes[i] -= required_descriptor_count[i];
				}
				++allocated_count;
				countdown_count = k_max_frames_unused;

				return allocated_descriptors.emplace_back(descriptor_set, required_descriptor_count);
			}

			vk::Result DescriptorPool::Free(DescriptorSet& descriptor_set)
			{
				EDGE_PROFILE_ZONE_SCOPED;

				if (descriptor_set.handle)
				{
					Handle(device).freeDescriptorSets(handle, 1u, &descriptor_set.handle);
					descriptor_set.handle = nullptr;

					--allocated_count;
					countdown_count = k_max_frames_unused;

					for (int32_t i = 0; i < k_max_pool_sizes_count; ++i)
					{
						avaliable_sizes[i] += descriptor_set.used_sizes[i];
					}

					return vk::Result::eSuccess;
				}

				return vk::Result::eIncomplete;
			}

			bool DescriptorPool::IsExpired() const
			{
				return countdown_count < 0;
			}

			CanAllocateResult DescriptorPool::CanAllocate(DescriptorSetLayout& layout)
			{
				auto& required_descriptor_count = layout.GetRequiredDescriptorCount();

				bool max_set_limit_reached = allocated_count >= k_max_sets;
				bool max_descriptor_limit_reached = !HaveEnoughtSizes(avaliable_sizes, required_descriptor_count);

				last_allocation_result = CanAllocateResult::Success;
				if (!max_descriptor_limit_reached && max_set_limit_reached)
					last_allocation_result = CanAllocateResult::MaxSetsLimit;
				else if (max_descriptor_limit_reached)
					last_allocation_result = CanAllocateResult::MaxSetsLimit;

				// TODO: implement flags and other

				return last_allocation_result;
			}
		}
	}
}