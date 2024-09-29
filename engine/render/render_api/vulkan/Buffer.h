#pragma once

#include <engine/render/render_api/vulkan/Allocator.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class Buffer;
			class VkRenderDevice;

			class BufferBuilder : public AllocationBuilder<BufferBuilder, vk::BufferCreateInfo>
			{
			public:
				BufferBuilder(vk::DeviceSize size);

				Buffer Build(VkRenderDevice& render_device) const;
				std::unique_ptr<Buffer> BuildUnique(VkRenderDevice& render_device) const;
				BufferBuilder& SetFlags(vk::BufferCreateFlags flags);
				BufferBuilder& SetImplicitSharingMode();
				BufferBuilder& SetSharingMode(vk::SharingMode sharing_mode);
				BufferBuilder& SetUsage(vk::BufferUsageFlagBits usage);
			};

			class Buffer final : public Allocation, public VulkanResource<vk::Buffer>
			{
			public:
				Buffer() = delete;
				Buffer(const Buffer&) = delete;
				Buffer(Buffer&& other) = default;
				Buffer& operator=(const Buffer&) = delete;
				Buffer& operator=(Buffer&&) = default;

				Buffer(VkRenderDevice& render_device, BufferBuilder const& builder);
				~Buffer();

				/**
				 * @return Return the buffer's device address (note: requires that the buffer has been created with the VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT usage fla)
				 */
				uint64_t GetDeviceAddress() const;

				/**
				 * @return The size of the buffer
				 */
				vk::DeviceSize GetSize() const;

			private:
				vk::DeviceSize size{ 0ull };
			};
		}
	}
}