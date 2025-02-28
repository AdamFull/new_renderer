#include "VkBuffer.h"
#include "VkPhysicalDevice.h"
#include "VkDevice.h"
#include "VkDebug.h"

#include "../../util/util.h"
#include "../../util/memory.hpp"

namespace edge
{
    namespace gfx
    {
        namespace vulkan
        {
            VulkanBuffer::VulkanBuffer(Allocator& allocator, const BufferCreateInfo& create_info) :
                VulkanResource{ nullptr, allocator.GetAllocationCallbacks() },
                Allocation(allocator),
                usage_flags{ create_info.usage }
            {
                uint64_t minimal_alignment{ 1ull };
                vk::BufferCreateInfo buffer_create_info{};

                auto& device = allocator.GetDevice();
                auto& physical = device.GetGpu();
                auto& properties = physical.GetProperties();

                if (create_info.usage & BufferUsageFlag::eCopy)
                {
                    buffer_create_info.usage |= vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
                }

                if (create_info.usage & BufferUsageFlag::eVertex)
                {
                    buffer_create_info.usage |= vk::BufferUsageFlagBits::eVertexBuffer;
                    minimal_alignment = std::max<uint64_t>(minimal_alignment, 4ull);
                }

                if (create_info.usage & BufferUsageFlag::eIndex)
                {
                    buffer_create_info.usage |= vk::BufferUsageFlagBits::eIndexBuffer;
                    minimal_alignment = std::max<uint64_t>(minimal_alignment, 4ull);
                }

                if (create_info.usage & BufferUsageFlag::eUniform)
                {
                    buffer_create_info.usage |= vk::BufferUsageFlagBits::eUniformBuffer;
                    minimal_alignment = std::lcm(properties.limits.minUniformBufferOffsetAlignment, properties.limits.nonCoherentAtomSize);
                }

                if (create_info.usage & BufferUsageFlag::eStorage)
                {
                    buffer_create_info.usage |= vk::BufferUsageFlagBits::eStorageBuffer;
                    minimal_alignment = std::max(minimal_alignment, properties.limits.minStorageBufferOffsetAlignment);
                }

                if (create_info.usage & BufferUsageFlag::eIndirect)
                {
                    buffer_create_info.usage |= vk::BufferUsageFlagBits::eIndirectBuffer;
                }

                if (create_info.usage & BufferUsageFlag::eHasDeviceAddress)
                {
                    buffer_create_info.usage |= vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;
                }

                if (create_info.usage & BufferUsageFlag::eGpuOnly)
                    alloc_create_info.usage = vma::MemoryUsage::eGpuOnly;
                else if (create_info.usage & BufferUsageFlag::eCpuToGpu)
                    alloc_create_info.usage = vma::MemoryUsage::eCpuToGpu;
                else
                    alloc_create_info.usage = vma::MemoryUsage::eCpuOnly;

                buffer_create_info.size = aligned_size(create_info.size, minimal_alignment) * create_info.count;
                size = buffer_create_info.size;

                buffer_create_info.sharingMode = vk::SharingMode::eConcurrent;

                auto family_count = device.GetQueueCount();

                std::array<uint32_t, 8u> queue_families{};
                std::iota(queue_families.begin(), queue_families.begin() + family_count, 0u);
                buffer_create_info.queueFamilyIndexCount = family_count;
                buffer_create_info.pQueueFamilyIndices = &queue_families[0];

                handle = allocator.CreateBuffer(*this, buffer_create_info);
            }

            VulkanBuffer::~VulkanBuffer()
            {
                allocator.DestroyBuffer(*this, handle);
            }

            void VulkanBuffer::SetResourceName(std::string_view name) const noexcept
            {
                auto& device = allocator.GetDevice();
                auto& debug_utils = device.GetDebugUtils();
                debug_utils.SetDebugName(handle, name);
            }

            auto VulkanBuffer::Map() -> uint8_t*
            {
                return MapMemory();
            }

            void VulkanBuffer::Unmap() noexcept
            {
                UnmapMemory();
            }

            auto VulkanBuffer::Update(const void* data, uint64_t size, uint64_t offset) -> uint64_t
            {
                if (persistent)
                {
                    engine::util::Copy(mapped_data, offset, data, size, 0ull);
                    Flush();
                }
                else
                {
                    MapMemory();
                    engine::util::Copy(mapped_data, offset, data, size, 0ull);
                    Flush();
                    UnmapMemory();
                }
                return size;
            }

            auto VulkanBuffer::GetSize() const noexcept -> uint64_t
            {
                return size;
            }

            auto VulkanBuffer::GetAddress() const -> uint64_t
            {
                auto& device = allocator.GetDevice();
                return Handle(device).getBufferAddressKHR(handle);
            }

            auto VulkanBuffer::GetUsageFlags() const noexcept -> BufferUsageFlags
            {
                return usage_flags;
            }
        }
    }
}