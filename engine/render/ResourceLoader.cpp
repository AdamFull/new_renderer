#include "ResourceLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <ktx.h>
#include "native_format/vk_format.h"

#include <util/Logger.h>

#include <shader_compiler/compiler_util.h>

#include <engine/filesystem/vfs_helper.h>

using namespace engine;
using namespace engine::filesystem;
using namespace engine::render;

EApiDataFormat lookup_api_format(u32 format)
{
	static const utl::hash_map<u32, EApiDataFormat> format_lookup{
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, EApiDataFormat::eD32SFLOAT_8UINT },
		{ VK_FORMAT_D24_UNORM_S8_UINT, EApiDataFormat::eD24UNORM_8UINT },
		{ VK_FORMAT_D16_UNORM_S8_UINT, EApiDataFormat::eD16UNORM_8UINT },
		{ VK_FORMAT_D32_SFLOAT, EApiDataFormat::eD32SFLOAT },

		{ VK_FORMAT_BC1_RGB_UNORM_BLOCK, EApiDataFormat::eBC1RGB_UNORM },
		{ VK_FORMAT_BC1_RGB_SRGB_BLOCK, EApiDataFormat::eBC1RGB_SRGB },
		{ VK_FORMAT_BC1_RGBA_UNORM_BLOCK, EApiDataFormat::eBC1RGBA_UNORM },
		{ VK_FORMAT_BC1_RGBA_SRGB_BLOCK, EApiDataFormat::eBC1RGBA_SRGB },
		{ VK_FORMAT_BC2_UNORM_BLOCK, EApiDataFormat::eBC2_UNORM },
		{ VK_FORMAT_BC2_SRGB_BLOCK, EApiDataFormat::eBC2_SRGB },
		{ VK_FORMAT_BC3_UNORM_BLOCK, EApiDataFormat::eBC_UNORM },
		{ VK_FORMAT_BC3_SRGB_BLOCK, EApiDataFormat::eBC3_SRGB },
		{ VK_FORMAT_BC4_UNORM_BLOCK, EApiDataFormat::eBC4_UNORM },
		{ VK_FORMAT_BC4_SNORM_BLOCK, EApiDataFormat::eBC4_SNORM },
		{ VK_FORMAT_BC5_UNORM_BLOCK, EApiDataFormat::eBC5_UNORM },
		{ VK_FORMAT_BC5_SNORM_BLOCK, EApiDataFormat::eBC5_SNORM },
		{ VK_FORMAT_BC6H_UFLOAT_BLOCK, EApiDataFormat::eBC6H_UFLOAT },
		{ VK_FORMAT_BC6H_SFLOAT_BLOCK, EApiDataFormat::eBC6H_SFLOAT },
		{ VK_FORMAT_BC7_UNORM_BLOCK, EApiDataFormat::eBC7_UNORM },
		{ VK_FORMAT_BC7_SRGB_BLOCK, EApiDataFormat::eBC7_SRGB },

		{ VK_FORMAT_D16_UNORM, EApiDataFormat::eD16UNORM },
		{ VK_FORMAT_R8G8B8A8_SRGB, EApiDataFormat::eRGBA8_SRGB },

		{ VK_FORMAT_R8_UNORM, EApiDataFormat::eR8_UNORM },
		{ VK_FORMAT_R8G8_UNORM, EApiDataFormat::eRG8_UNORM },
		{ VK_FORMAT_R8G8B8_UNORM, EApiDataFormat::eRGB8_UNORM },
		{ VK_FORMAT_R8G8B8A8_UNORM, EApiDataFormat::eRGBA8_UNORM },

		{ VK_FORMAT_R16_UNORM, EApiDataFormat::eR16_UNORM },
		{ VK_FORMAT_R16G16_UNORM, EApiDataFormat::eRG16_UNORM },
		{ VK_FORMAT_R16G16B16_UNORM, EApiDataFormat::eRGB16_UNORM },
		{ VK_FORMAT_R16G16B16A16_UNORM, EApiDataFormat::eRGBA16_UNORM },

		{ VK_FORMAT_R32_UINT, EApiDataFormat::eR32_UINT },
		{ VK_FORMAT_R32G32_UINT, EApiDataFormat::eRG32_UINT },
		{ VK_FORMAT_R32G32B32_UINT, EApiDataFormat::eRGB32_UINT },
		{ VK_FORMAT_R32G32B32A32_UINT, EApiDataFormat::eRGBA32_UINT },

		{ VK_FORMAT_R16_SFLOAT, EApiDataFormat::eR16_SFLOAT },
		{ VK_FORMAT_R16G16_SFLOAT, EApiDataFormat::eRG16_SFLOAT },
		{ VK_FORMAT_R16G16B16_SFLOAT, EApiDataFormat::eRGB16_SFLOAT },
		{ VK_FORMAT_R16G16B16A16_SFLOAT, EApiDataFormat::eRGBA16_SFLOAT },

		{ VK_FORMAT_R32_SFLOAT, EApiDataFormat::eR32_SFLOAT },
		{ VK_FORMAT_R32G32_SFLOAT, EApiDataFormat::eRG32_SFLOAT },
		{ VK_FORMAT_R32G32B32_SFLOAT, EApiDataFormat::eRGB32_SFLOAT },
		{ VK_FORMAT_R32G32B32A32_SFLOAT, EApiDataFormat::eRGBA32_SFLOAT },

		{ VK_FORMAT_B10G11R11_UFLOAT_PACK32, EApiDataFormat::eR11G11B10_UFLOAT },
		{ VK_FORMAT_A2B10G10R10_UNORM_PACK32, EApiDataFormat::eR10G10B10A2_UNORM },
	};

	if (auto found = format_lookup.find(format); found != format_lookup.end())
		return found->second;

	return EApiDataFormat::eUnknown;
}

EApiDataFormat get_ktx_format(ktxTexture* This)
{
	if (This->classId == ktxTexture2_c)
		return lookup_api_format(((ktxTexture2*)This)->vkFormat);

	auto* ktx1 = (ktxTexture1*)This;
	u32 vk_format{};
	vk_format = vkGetFormatFromOpenGLInternalFormat(ktx1->glInternalformat);
	if (vk_format == 0u)
		vk_format = vkGetFormatFromOpenGLFormat(ktx1->glFormat, ktx1->glType);
	return lookup_api_format(vk_format);
}

u32 get_ktx_api_specific_format(ktxTexture* This)
{
	return 0;
}

void CResourceLoader::Initialize(IVirtualFileSystemInterface* vfs)
{
	m_pVFS = vfs;

	if (!vfs->ReadBson("/root/embed/rendering/shaders/shader_cache.shc", m_shaderCache))
		log_error("Failed to load shader cache. ")
}

bool CResourceLoader::LoadTexture(const utl::vector<uint8_t>& data, EMimeType mime, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info, FTextureResourceHandle& handle)
{
	create_info.flags = ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eTransferable;
	handle.mime_type = mime;

	switch (mime)
	{
	case EMimeType::eJPG:
	case EMimeType::eJPEG:
	case EMimeType::ePNG:
	case EMimeType::eTGA:
	case EMimeType::eBMP:
	case EMimeType::ePSD:
	case EMimeType::eGIF:
	case EMimeType::eHDR:
	case EMimeType::ePIC:
	case EMimeType::ePNM:
		return LoadSTBFromMemory(data, create_info, upload_info, handle);
	case EMimeType::eKTX:
		return LoadKTXFromMemory(data, create_info, upload_info, handle);
	case EMimeType::eKTX2:
		return LoadKTX2FromMemory(data, create_info, upload_info, handle);
	case EMimeType::eDDS:
		return LoadDDSFromMemory(data, create_info, upload_info, handle);
	}

	return false;
}

void CResourceLoader::DestroyHandle(FTextureResourceHandle& handle)
{
	switch (handle.mime_type)
	{
	case EMimeType::eJPG:
	case EMimeType::eJPEG:
	case EMimeType::ePNG:
	case EMimeType::eTGA:
	case EMimeType::eBMP:
	case EMimeType::ePSD:
	case EMimeType::eGIF:
	case EMimeType::eHDR:
	case EMimeType::ePIC:
	case EMimeType::ePNM:
	{
		stbi_image_free(handle.handle);
	} break;
	case EMimeType::eKTX:
	case EMimeType::eKTX2:
	{
		ktxTexture_Destroy((ktxTexture*)handle.handle);
	} break;
	}

	handle.handle = nullptr;
}

bool CResourceLoader::LoadShaderData(const FShaderUploadInfo& load_info, utl::vector<FShaderCreationBlockInfo>& shader_stages)
{
	// Looking for shader library (container with shader code and with shader modules)
	auto found_library = m_shaderCache.libraries.find(load_info.libraryId);
	if (found_library == m_shaderCache.libraries.end())
		return false;

	shader_compiler::FCompiledLibrary& shader_library = found_library->second;

	// Looking for shader module (shader type with shader association links)
	auto found_shader_module = shader_library.modules.find(load_info.shaderId);
	if (found_shader_module == shader_library.modules.end())
		return false;

	const utl::vector<shader_compiler::FShaderModule>& shader_modules = found_shader_module->second;

	for (const auto& shader_module : shader_modules)
	{
		auto pool_name = shader_compiler::get_shader_type(shader_module.type);

		auto found_shader_pool = shader_library.shaders.find(pool_name);
		if (found_shader_pool == shader_library.shaders.end())
		{
			log_error("Shader pool for module named \"{}\" was not found.", load_info.shaderId);
			return false;
		}

		auto& shader_pool = found_shader_pool->second;

		auto found_shader_code = shader_pool.find(shader_module.link);
		if (found_shader_code == shader_pool.end())
			continue;

		shader_compiler::FShaderData& shader_data = found_shader_code->second;

		auto& shader_code_info = shader_stages.emplace_back();
		shader_code_info.stage = utl::cast<u32>(shader_module.type);

		if (shader_data.data.empty())
		{
			auto file_path = fs::path_append("/root/embed/rendering/shaders", shader_data.path);
			if (!m_pVFS->ReadFile(file_path, shader_data.data))
			{
				log_error("Failed to load shader: {}.", shader_data.path);
				return false;
			}
		}

		shader_code_info.code = &shader_data.data;
	}

	return true;
}

bool CResourceLoader::LoadSTBFromMemory(const utl::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info, FTextureResourceHandle& handle)
{
	int width{ 0 }, height{ 0 }, channels{ 0 };
	auto rawdata = stbi_load_from_memory(data.data(), data.size(), &width, &height, &channels, STBI_rgb_alpha);

	create_info.SetFixedSize2D(utl::cast<u32>(width), utl::cast<u32>(height));
	create_info.format = EApiDataFormat::eRGBA8_UNORM;
	auto data_size = utl::cast<u32>(width * height * 4ull);

	upload_info.upload_data = std::span{ rawdata, data_size };

	create_info.flags |= ETextureCreateFlagBits::e2D;

	create_info.mipLevels = utl::cast<u32>(std::floor(std::log2((std::max)(width, height)))) + 1u;

	handle.handle = rawdata;

	return true;
}

bool CResourceLoader::LoadKTXFromMemory(const utl::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info, FTextureResourceHandle& handle)
{
	ktxTexture* kTexture{ nullptr };
	auto ktxresult = ktxTexture_CreateFromMemory(data.data(), data.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &kTexture);
	if (ktxresult != KTX_SUCCESS)
		return false;

	if (kTexture->isArray)
		create_info.flags |= ETextureCreateFlagBits::eArray;
	if (kTexture->isCubemap)
		create_info.flags |= ETextureCreateFlagBits::eCubemap;
	
	create_info.SetFixedSize3D(kTexture->baseWidth, kTexture->baseHeight, kTexture->baseDepth);
	
	if (kTexture->numDimensions == 1u)
		create_info.flags |= ETextureCreateFlagBits::e1D;
	else if (kTexture->numDimensions == 2u)
		create_info.flags |= ETextureCreateFlagBits::e2D;
	else if (kTexture->numDimensions == 3u)
		create_info.flags |= ETextureCreateFlagBits::e3D;

	create_info.mipLevels = kTexture->numLevels;
	create_info.arrayLayers = kTexture->numLayers;

	upload_info.upload_data = std::span{ kTexture->pData, kTexture->dataSize };

	// TODO: get api format
	create_info.format = get_ktx_format(kTexture);

	auto num_layers = kTexture->isCubemap ? kTexture->numFaces : kTexture->numLayers;
	for (u32 layer = 0; layer < num_layers; layer++)
	{
		auto& mip_layer = upload_info.mip_layer_offsets.emplace_back();
		for (u32 level = 0u; level < kTexture->numLevels; level++)
		{
			auto& mip_offset = mip_layer.emplace_back();
			ktxresult = ktxTexture_GetImageOffset(kTexture, level, 0, layer, &mip_offset);
		}
	}

	handle.handle = kTexture;

	return true;
}

bool CResourceLoader::LoadKTX2FromMemory(const utl::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info, FTextureResourceHandle& handle)
{
	ktxTexture2* kTexture{ nullptr };
	auto ktxresult = ktxTexture_CreateFromMemory(data.data(), data.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, (ktxTexture**)&kTexture);
	if (ktxresult != KTX_SUCCESS)
		return false;

	create_info.flags |= ETextureCreateFlagBits::eTransferable | ETextureCreateFlagBits::eSamplable;

	if (kTexture->isArray)
		create_info.flags |= ETextureCreateFlagBits::eArray;
	if (kTexture->isCubemap)
		create_info.flags |= ETextureCreateFlagBits::eCubemap;

	create_info.SetFixedSize3D(kTexture->baseWidth, kTexture->baseHeight, kTexture->baseDepth);

	if (kTexture->numDimensions == 1u)
		create_info.flags |= ETextureCreateFlagBits::e1D;
	else if (kTexture->numDimensions == 2u)
		create_info.flags |= ETextureCreateFlagBits::e2D;
	else if (kTexture->numDimensions == 3u)
		create_info.flags |= ETextureCreateFlagBits::e3D;

	create_info.mipLevels = kTexture->numLevels;
	create_info.arrayLayers = kTexture->numLayers;

	if (ktxTexture2_NeedsTranscoding(kTexture))
		ktxresult = ktxTexture2_TranscodeBasis(kTexture, KTX_TTF_BC7_RGBA, 0);

	upload_info.upload_data = std::span{ kTexture->pData, kTexture->dataSize };

	create_info.format = lookup_api_format(kTexture->vkFormat);

	auto num_layers = kTexture->isCubemap ? kTexture->numFaces : kTexture->numLayers;
	for (u32 layer = 0; layer < num_layers; layer++)
	{
		auto& mip_layer = upload_info.mip_layer_offsets.emplace_back();
		for (u32 level = 0u; level < kTexture->numLevels; level++)
		{
			auto& mip_offset = mip_layer.emplace_back();
			ktxresult = ktxTexture_GetImageOffset((ktxTexture*)kTexture, level, 0, layer, &mip_offset);
		}
	}

	handle.handle = kTexture;

	return true;
}

bool CResourceLoader::LoadDDSFromMemory(const utl::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info, FTextureResourceHandle& handle)
{
	return false;
}