#pragma once

#include <GFX.h>

namespace gfx
{
	namespace vulkan
	{
		class Device;

		class VulkanTexture final : public Texture
		{
		public:
			VulkanTexture(Device& device, const TextureCreateInfo& create_info);
			VulkanTexture(const VulkanTexture&) = delete;
			VulkanTexture(VulkanTexture&& other);
			~VulkanTexture();

			VulkanTexture& operator=(const VulkanTexture&) = delete;
			VulkanTexture& operator=(VulkanTexture&&) = delete;
		private:
			Device& device;
		};
	}
}