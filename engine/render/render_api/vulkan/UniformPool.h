#pragma once

#include <vulkan/vulkan.hpp>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class RenderDevice;
			class Buffer;

			struct UniformBlockDesc
			{
				Buffer& buffer;
				vk::DeviceSize offset;
				vk::DeviceSize size;
			};

			class UniformPoolFrame final
			{
			public:
				UniformPoolFrame(RenderDevice& render_device, vk::DeviceSize whole_size, vk::DeviceSize alignment);

				UniformPoolFrame(const UniformPoolFrame&) = delete;
				UniformPoolFrame(UniformPoolFrame&& other);
				~UniformPoolFrame() = default;

				UniformPoolFrame& operator=(const UniformPoolFrame&) = delete;
				UniformPoolFrame& operator=(UniformPoolFrame&&) = delete;

				bool IsExpired() const;
				void Reset() const;
				bool HasEnoughtSpace(vk::DeviceSize requested_size) const;
				UniformBlockDesc RequestBlock(vk::DeviceSize requested_size);
			private:
				std::unique_ptr<Buffer> buffer;
				mutable vk::DeviceSize size_remain{ 0ull };

				mutable int32_t expired_countdown{ 0 };
			};

			class UniformPool final
			{
			public:
				UniformPool(RenderDevice& render_device, vk::DeviceSize frame_size = 4096ull);

				UniformPool(const UniformPool&) = delete;
				UniformPool(UniformPool&& other);
				~UniformPool() = default;

				UniformPool& operator=(const UniformPool&) = delete;
				UniformPool& operator=(UniformPool&&) = delete;

				void Reset();
				UniformBlockDesc RequestBlock(vk::DeviceSize requested_size);
			private:
				RenderDevice& render_device;

				vk::DeviceSize minimal_alignment{ 1ull };
				vk::DeviceSize frame_size{ 4096ull };
				std::vector<std::unique_ptr<UniformPoolFrame>> pool_frames;
			};
		}
	}
}