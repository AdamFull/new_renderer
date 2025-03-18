#include "gfx_texture_loader.h"

#include "../../fs/fs_helper.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <ktx.h>

#include "../gfx_context.h"

namespace edge
{
	namespace gfx
	{
		class STBImageLoader final : public ITextureLoader
		{
		public:
			~STBImageLoader() override
			{
				if (image_data)
				{
					stbi_image_free(image_data);
				}
			}

			auto Load(std::span<const uint8_t> data, TextureCreateInfo& create_info) -> bool override
			{
				image_data = stbi_load_from_memory(data.data(), data.size(), &image_width, &image_height, &image_channel_count, STBI_rgb_alpha);

				create_info.usage |= gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eStorage | gfx::TextureUsageFlag::eTransferable;
				create_info.width = image_width;
				create_info.height = image_height;
				//create_info.format = image_channel_count == 4 ? gfx::Format::eR8G8B8A8Unorm : gfx::Format::eR8G8B8Unorm;
				create_info.format = gfx::Format::eR8G8B8A8Unorm;
				//create_info.levels = static_cast<uint32_t>(std::floor(std::log2((std::max)(image_width, image_height)))) + 1u;
				create_info.levels = 1;

				return image_data;
			}

			auto IsNeedToGenerateMips() const noexcept -> bool override
			{
				return true;
			}

			auto GetSize() -> uint64_t override
			{
				return image_width * image_height * 4ull;
			}

			auto Upload(ICommandList* cmd_list, IBuffer* src_buffer, uint64_t src_offset, ITexture* dst_texture) -> void override
			{
				TextureCopyRegion copy_region{};
				copy_region.buffer_offset = src_offset;
				copy_region.size_x = image_width;
				copy_region.size_y = image_height;

				gfx::CopyTextureInfo copy_texture{};
				copy_texture.src_buffer = src_buffer;
				copy_texture.dst_texture = dst_texture;
				copy_texture.copy_regions = { &copy_region, 1 };

				auto* src_memory = src_buffer->Map();
				memcpy(src_memory + src_offset, image_data, GetSize());

				cmd_list->CopyTexture(copy_texture);
			}
		private:
			uint8_t* image_data{ nullptr };
			int32_t image_width{ 1 };
			int32_t image_height{ 1 };
			int32_t image_channel_count{ 1 };
		};

		class KTXImageLoader final : public ITextureLoader
		{
		public:
			~KTXImageLoader() override
			{
				if (texture)
				{
					ktxTexture_Destroy(texture);
				}
			}

			auto Load(std::span<const uint8_t> data, TextureCreateInfo& create_info) -> bool override
			{
				auto ktxresult = ktxTexture_CreateFromMemory(data.data(), data.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture);
				assert(ktxresult == KTX_SUCCESS && "FAILED TO OPEN KTX IMAGE");
				if (ktxresult != KTX_SUCCESS)
					return false;

				if (texture->isCubemap)
					create_info.usage |= gfx::TextureUsageFlag::eCubemap;

				create_info.usage |= gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eTransferable;
				create_info.width = texture->baseWidth;
				create_info.height = texture->baseHeight;
				create_info.depth = texture->baseDepth;

				create_info.levels = texture->numLevels;
				create_info.layers = texture->numLayers;

				if (texture->generateMipmaps)
				{
					create_info.usage |= gfx::TextureUsageFlag::eCubemap;
					create_info.levels = static_cast<uint32_t>(std::floor(std::log2((std::max)(create_info.width, create_info.height)))) + 1u;
				}

				switch (texture->classId)
				{
				case ktxTexture1_c: {
					ktxTexture1* ktxtexture1 = (ktxTexture1*)texture;

					//uint32_t vk_format = vkGetFormatFromOpenGLInternalFormat(ktxtexture1->glInternalformat);
					//if (vk_format == 0u)
					//{
					//	vk_format = vkGetFormatFromOpenGLFormat(ktxtexture1->glFormat, ktxtexture1->glType);
					//}
					//
					//create_info.format = gfx::FormatFromVk(vk_format);
				} break;
				case ktxTexture2_c: {
					ktxTexture2* ktxtexture2 = (ktxTexture2*)texture;

					if (ktxTexture2_NeedsTranscoding(ktxtexture2))
					{
						ktxresult = ktxTexture2_TranscodeBasis(ktxtexture2, KTX_TTF_BC7_RGBA, 0);
					}

					create_info.format = gfx::FormatFromVk(ktxtexture2->vkFormat);
				} break;
				}

				return true;
			}

			auto IsNeedToGenerateMips() const noexcept -> bool override
			{
				return texture->generateMipmaps;
			}

			auto GetSize() -> uint64_t override
			{
				return texture->dataSize;
			}

			auto Upload(ICommandList* cmd_list, IBuffer* src_buffer, uint64_t src_offset, ITexture* dst_texture) -> void override
			{
				std::vector<TextureCopyRegion> copy_regions{};

				gfx::CopyTextureInfo copy_texture{};
				copy_texture.src_buffer = src_buffer;
				copy_texture.dst_texture = dst_texture;

				auto* src_memory = src_buffer->Map();
				memcpy(src_memory + src_offset, texture->pData, texture->dataSize);

				// Collect image copy regions
				auto num_layers = texture->isCubemap ? texture->numFaces : texture->numLayers;
				for (int32_t layer = 0; layer < num_layers; layer++)
				{
					for (int32_t level = 0; level < texture->numLevels; level++)
					{
						auto& region = copy_regions.emplace_back(src_offset, 0, 0, 0,
							(texture->baseWidth >> level),
							(texture->baseHeight >> level), 1,
							layer, texture->numLayers, level);

						auto ktxresult = ktxTexture_GetImageOffset(texture, level, 0, layer, &region.buffer_offset);
						if (ktxresult != KTX_SUCCESS)
							return;

						region.buffer_offset += src_offset;
					}
				}

				copy_texture.copy_regions = copy_regions;
				cmd_list->CopyTexture(copy_texture);
			}
		private:
			ktxTexture* texture{ nullptr };
		};

		auto MakeTextureLoader(std::string_view path) -> std::unique_ptr<ITextureLoader>
		{
			std::unique_ptr<ITextureLoader> texture_loader;

			auto ext = fs::GetExt(path);
			auto ext_hash = murmur_hash(ext.data(), ext.size(), 0);
			switch (ext_hash)
			{
			case fs::format::ktx:
			case fs::format::ktx2: { texture_loader = std::make_unique<KTXImageLoader>(); } break;
			default: { texture_loader = std::make_unique<STBImageLoader>(); } break;
			}

			return texture_loader;
		}
	}
}