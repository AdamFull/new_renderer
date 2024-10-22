#pragma once

#include <GFX.h>

namespace gfx
{
	namespace vulkan
	{
		class Device;
		class Allocator;
		class Image;
		class ImageView;

		class VulkanTexture final : public Texture
		{
		public:
			VulkanTexture(Allocator& allocator, const TextureCreateInfo& create_info);
			VulkanTexture(const VulkanTexture&) = delete;
			VulkanTexture(VulkanTexture&& other);
			~VulkanTexture();

			VulkanTexture& operator=(const VulkanTexture&) = delete;
			VulkanTexture& operator=(VulkanTexture&&) = delete;
		private:
			Device& device;

			std::unique_ptr<Image> image{};
			std::unique_ptr<ImageView> srv{};
		};
	}
}