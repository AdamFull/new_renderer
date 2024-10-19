#include <engine/render/render_api/vulkan/UniformPool.h>
#include <engine/render/render_api/vulkan/PhysicalDevice.h>
#include <engine/render/render_api/vulkan/VkRenderDevice.h>
#include <engine/render/render_api/vulkan/Buffer.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			constexpr int32_t k_max_frames_idle{ 300 };

			inline vk::DeviceSize CalculateAlignedSize(vk::DeviceSize allocation_size, vk::DeviceSize minimal_alignment)
			{
				if (minimal_alignment > 0ull)
					return (allocation_size + minimal_alignment - 1ull) & ~(minimal_alignment - 1ull);
				return allocation_size;
			}

			UniformPoolFrame::UniformPoolFrame(RenderDevice& render_device, vk::DeviceSize whole_size, vk::DeviceSize alignment)
			{
				vk::BufferUsageFlags usage_flags{ vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst };

				auto builder = BufferBuilder(whole_size, 1ull, alignment);
				builder.SetVmaUsage(vma::MemoryUsage::eCpuToGpu);
				builder.SetUsage(usage_flags);
				builder.SetSharingMode(vk::SharingMode::eConcurrent);

				buffer = builder.BuildUnique(render_device);
				size_remain = buffer->GetSize();
				expired_countdown = k_max_frames_idle;
			}

			UniformPoolFrame::UniformPoolFrame(UniformPoolFrame&& other) :
				buffer{ std::exchange(other.buffer, {}) },
				size_remain{ std::exchange(other.size_remain, {}) },
				expired_countdown{ std::exchange(other.expired_countdown, {}) }
			{
			}

			bool UniformPoolFrame::IsExpired() const
			{
				return expired_countdown < 0;
			}

			void UniformPoolFrame::Reset() const
			{
				size_remain = buffer->GetSize();
				--expired_countdown;
			}

			bool UniformPoolFrame::HasEnoughtSpace(vk::DeviceSize requested_size) const
			{
				return static_cast<int64_t>(size_remain) - static_cast<int64_t>(requested_size) > 0;
			}

			UniformBlockDesc UniformPoolFrame::RequestBlock(vk::DeviceSize requested_size)
			{
				UniformBlockDesc block_desc{ *buffer, buffer->GetSize() - size_remain, requested_size };

				expired_countdown = k_max_frames_idle;
				size_remain -= requested_size;

				return block_desc;
			}



			UniformPool::UniformPool(RenderDevice& render_device, vk::DeviceSize frame_size) :
				render_device{ render_device },
				frame_size{ frame_size }
			{
				auto& physical = render_device.GetPhysicalDevice();
				const auto& properties = physical.GetProperties();
				minimal_alignment = std::lcm(properties.limits.minUniformBufferOffsetAlignment, properties.limits.nonCoherentAtomSize);

				pool_frames.emplace_back(std::make_unique<UniformPoolFrame>(render_device, frame_size, minimal_alignment));
			}

			UniformPool::UniformPool(UniformPool&& other) :
				render_device{ other.render_device },
				frame_size{ std::exchange(other.frame_size, {}) },
				pool_frames{ std::move(other.pool_frames) }
			{
			}

			void UniformPool::Reset()
			{
				auto to_delete = std::remove_if(pool_frames.begin(), pool_frames.end(), [](const std::unique_ptr<UniformPoolFrame>& frame)
					{
						frame->Reset();
						return frame->IsExpired();
					});

				if (to_delete != pool_frames.end())
				{
					pool_frames.erase(to_delete);
				}
			}

			UniformBlockDesc UniformPool::RequestBlock(vk::DeviceSize requested_size)
			{
				//minUniformBufferOffsetAlignment
				vk::DeviceSize aligned_size = CalculateAlignedSize(requested_size, minimal_alignment);
				auto found = std::find_if(pool_frames.begin(), pool_frames.end(), [&aligned_size](const std::unique_ptr<UniformPoolFrame>& frame)
					{
						return frame->HasEnoughtSpace(aligned_size);
					});
				;
				if (found != pool_frames.end())
				{
					return (*found)->RequestBlock(aligned_size);
				}

				auto& new_pool = pool_frames.emplace_back(std::make_unique<UniformPoolFrame>(render_device, frame_size, minimal_alignment));
				return new_pool->RequestBlock(aligned_size);
			}
		}
	}
}