#pragma once

#include <memory>
#include <engine/render/render_api/vulkan/VkResourceStructures.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class Image;
			class ImageView;
			class VkRenderDevice;

			struct TextureProperties
			{
				vk::Format format{};
				ETextureCreateFlags flags{ 0u };
				FTextureSizeInfo size{};

				uint32_t mip_levels{ 1u };
				uint32_t array_layers{ 1u };

				uint32_t total_mips{ 1u };
				uint32_t total_layers{ 1u };

				vk::SampleCountFlagBits sample_count{ vk::SampleCountFlagBits::e1 };
			};

			class Texture
			{
			public:
				Texture(VkRenderDevice& render_device, const FVkTextureCreateInfo& create_info);
				~Texture();

				Image& GetImage();
				const Image& GetImage() const;
			private:
				TextureProperties properties;

				std::unique_ptr<Image> image;

				std::unique_ptr<ImageView> base;
				std::vector<std::unique_ptr<ImageView>> mips;
			};
		}
	}
}