#include "gfx_texture_loader.h"

#include "../../fs/fs_helper.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <ktx.h>

#include "gfx_staging_allocator.h"
#include "../gfx_context.h"
#include "../../fs/fs_virtual_filesystem.h"

namespace edge
{
	namespace gfx
	{
		auto TextureLoader::GetTexture() -> ITexture*
		{
			return texture;
		}

		class STBTextureLoader final : public TextureLoader
		{
		public:
			STBTextureLoader(IContext* gfx_context, std::span<const uint8_t> data)
			{
				int32_t image_width, image_height, image_channel_count;
				image_data = stbi_load_from_memory(data.data(), data.size(), &image_width, &image_height, &image_channel_count, STBI_rgb_alpha);

				texture_ci.usage |= gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eStorage | gfx::TextureUsageFlag::eTransferable;
				texture_ci.width = image_width;
				texture_ci.height = image_height;
				//texture_ci.format = image_channel_count == 4 ? gfx::Format::eR8G8B8A8Unorm : gfx::Format::eR8G8B8Unorm;
				texture_ci.format = gfx::Format::eR8G8B8A8Unorm;
				//texture_ci.levels = static_cast<uint32_t>(std::floor(std::log2((std::max)(image_width, image_height)))) + 1u;
				texture_ci.levels = 1;

				texture = gfx_context->CreateTexture(texture_ci);
			}

			~STBTextureLoader() override
			{
				if (image_data)
				{
					stbi_image_free(image_data);
				}
			}

			auto Upload(ICommandList* cmd_list, StagingBufferAllocator& allocator) -> void override
			{
				auto texture_size = texture_ci.width * texture_ci.height * 4ull;

				auto block_align = GetFormatBlockAlign(texture->GetFormat());
				auto [src_buffer, src_offset] = allocator.Allocate(texture_size, block_align);

				TextureCopyRegion copy_region{};
				copy_region.buffer_offset = src_offset;
				copy_region.size_x = texture_ci.width;
				copy_region.size_y = texture_ci.height;

				gfx::CopyTextureInfo copy_texture{};
				copy_texture.src_buffer = src_buffer;
				copy_texture.dst_texture = texture;
				copy_texture.copy_regions = { &copy_region, 1 };

				auto* src_memory = src_buffer->Map();
				memcpy(src_memory + src_offset, image_data, texture_size);

				cmd_list->CopyTexture(copy_texture);
			}
		private:
			uint8_t* image_data{ nullptr };
		};

		class KTXTextureLoader final : public TextureLoader
		{
		public:
			KTXTextureLoader(IContext* gfx_context, std::span<const uint8_t> data)
			{
				auto ktxresult = ktxTexture_CreateFromMemory(data.data(), data.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxtexture);
				assert(ktxresult == KTX_SUCCESS && "FAILED TO OPEN KTX IMAGE");
				if (ktxresult != KTX_SUCCESS)
					return;

				if (ktxtexture->isCubemap)
					texture_ci.usage |= gfx::TextureUsageFlag::eCubemap;

				texture_ci.usage |= gfx::TextureUsageFlag::eSamplable | gfx::TextureUsageFlag::eTransferable;
				texture_ci.width = ktxtexture->baseWidth;
				texture_ci.height = ktxtexture->baseHeight;
				texture_ci.depth = ktxtexture->baseDepth;

				texture_ci.levels = ktxtexture->numLevels;
				texture_ci.layers = ktxtexture->numLayers;

				if (ktxtexture->generateMipmaps)
				{
					texture_ci.usage |= gfx::TextureUsageFlag::eCubemap;
					texture_ci.levels = static_cast<uint32_t>(std::floor(std::log2((std::max)(texture_ci.width, texture_ci.height)))) + 1u;
				}

				switch (ktxtexture->classId)
				{
				case ktxTexture1_c: {
					ktxTexture1* ktxtexture1 = (ktxTexture1*)ktxtexture;

					//uint32_t vk_format = vkGetFormatFromOpenGLInternalFormat(ktxtexture1->glInternalformat);
					//if (vk_format == 0u)
					//{
					//	vk_format = vkGetFormatFromOpenGLFormat(ktxtexture1->glFormat, ktxtexture1->glType);
					//}
					//
					//texture_ci.format = gfx::FormatFromVk(vk_format);
				} break;
				case ktxTexture2_c: {
					ktxTexture2* ktxtexture2 = (ktxTexture2*)ktxtexture;

					if (ktxTexture2_NeedsTranscoding(ktxtexture2))
					{
						ktxresult = ktxTexture2_TranscodeBasis(ktxtexture2, KTX_TTF_BC7_RGBA, 0);
					}

					texture_ci.format = gfx::FormatFromVk(ktxtexture2->vkFormat);
				} break;
				}

				texture = gfx_context->CreateTexture(texture_ci);
			}

			~KTXTextureLoader() override
			{
				if (ktxtexture)
				{
					ktxTexture_Destroy(ktxtexture);
				}
			}

			auto Upload(ICommandList* cmd_list, StagingBufferAllocator& allocator) -> void override
			{
				std::vector<TextureCopyRegion> copy_regions{};

				auto block_align = GetFormatBlockAlign(texture->GetFormat());
				auto [src_buffer, src_offset] = allocator.Allocate(ktxtexture->dataSize, block_align);

				gfx::CopyTextureInfo copy_texture{};
				copy_texture.src_buffer = src_buffer;
				copy_texture.dst_texture = texture;

				auto* src_memory = src_buffer->Map();
				memcpy(src_memory + src_offset, ktxtexture->pData, ktxtexture->dataSize);

				// Collect image copy regions
				auto num_layers = ktxtexture->isCubemap ? ktxtexture->numFaces : ktxtexture->numLayers;
				for (int32_t layer = 0; layer < num_layers; layer++)
				{
					for (int32_t level = 0; level < ktxtexture->numLevels; level++)
					{
						auto& region = copy_regions.emplace_back(src_offset, 0, 0, 0,
							(ktxtexture->baseWidth >> level),
							(ktxtexture->baseHeight >> level), 1,
							layer, ktxtexture->numLayers, level);

						auto ktxresult = ktxTexture_GetImageOffset(ktxtexture, level, 0, layer, &region.buffer_offset);
						if (ktxresult != KTX_SUCCESS)
							return;

						region.buffer_offset += src_offset;
					}
				}

				copy_texture.copy_regions = copy_regions;
				cmd_list->CopyTexture(copy_texture);
			}
		private:
			ktxTexture* ktxtexture{ nullptr };
		};

		auto MakeTextureLoader(std::string_view path, IContext* gfx_context, fs::VirtualFilesystem& fs) -> std::unique_ptr<TextureLoader>
		{
			std::unique_ptr<TextureLoader> texture_loader;

			std::vector<uint8_t> data{};
			fs.ReadFile(path, data);

			auto ext = fs::GetExt(path);
			auto ext_hash = murmur_hash(ext.data(), ext.size(), 0);
			switch (ext_hash)
			{
			case fs::format::ktx:
			case fs::format::ktx2: { texture_loader = std::make_unique<KTXTextureLoader>(gfx_context, data); } break;
			default: { texture_loader = std::make_unique<STBTextureLoader>(gfx_context, data); } break;
			}

			return texture_loader;
		}
	}
}