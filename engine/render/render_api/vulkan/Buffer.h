#pragma once

#include <engine/render/render_api/vulkan/Allocator.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class Buffer;
			class RenderDevice;

			class BufferBuilder : public AllocationBuilder<BufferBuilder, vk::BufferCreateInfo>
			{
			public:
				BufferBuilder(vk::DeviceSize allocation_size, vk::DeviceSize suballocation_count, vk::DeviceSize minimal_alignment);

				Buffer Build(RenderDevice& render_device) const;
				std::unique_ptr<Buffer> BuildUnique(RenderDevice& render_device) const;
				BufferBuilder& SetFlags(vk::BufferCreateFlags flags);
				BufferBuilder& SetImplicitSharingMode();
				BufferBuilder& SetSharingMode(vk::SharingMode sharing_mode);
				BufferBuilder& SetUsage(vk::BufferUsageFlags usage);

				vk::DeviceSize suballocation_count{ 0ull };
			};

			class Buffer final : public Allocation, public VulkanResource<vk::Buffer>
			{
			public:
				Buffer() = delete;
				Buffer(const Buffer&) = delete;
				Buffer(Buffer&& other) = default;
				Buffer& operator=(const Buffer&) = delete;
				Buffer& operator=(Buffer&&) = default;

				Buffer(RenderDevice& render_device, BufferBuilder const& builder);
				~Buffer();

				/**
				 * @brief Copy this buffer to other
				 * @param command_buffer The command buffer used for copy
				 * @param dst_buffer Destination buffer
				 * @param regions Buffer copy regions
				 */
				void CopyTo(vk::CommandBuffer& command_buffer, Buffer& dst_buffer, const std::vector<vk::BufferCopy2KHR>& regions);

				/**
				 * @brief Copy this buffer to image
				 * @param command_buffer The command buffer used for copy
				 * @param image Destination image
				 * @param layout Destination image layout
				 * @param regions Buffer copy regions
				 */
				void CopyTo(vk::CommandBuffer& command_buffer, vk::Image& image, vk::ImageLayout layout, const std::vector<vk::BufferImageCopy2KHR>& regions);

				/**
				 * @return Return the buffer's device address (note: requires that the buffer has been created with the VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT usage fla)
				 */
				uint64_t GetDeviceAddress() const;

				/**
				 * @return The size of the buffer
				 */
				vk::DeviceSize GetSize() const;

				vk::DeviceSize GetSuballocationSize() const;

			private:
				vk::DeviceSize size{ 0ull };
				vk::DeviceSize suballocation_count{ 0ull };
			};
		}
	}
}