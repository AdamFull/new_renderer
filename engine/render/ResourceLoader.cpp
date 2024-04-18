#include "ResourceLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <ktx.h>
#include "native_format/vk_format.h"

#include <logger/logger.h>

#include <shader_compiler/compiler_util.h>

#include <engine/filesystem/vfs_helper.h>

using namespace engine;
using namespace engine::filesystem;
using namespace engine::render;

u32 get_ktx_format(ktxTexture* This)
{
	if (This->classId == ktxTexture2_c)
		return ((ktxTexture2*)This)->vkFormat;

	auto* ktx1 = (ktxTexture1*)This;
	u32 vk_format{};
	vk_format = vkGetFormatFromOpenGLInternalFormat(ktx1->glInternalformat);
	if (vk_format == 0u)
		vk_format = vkGetFormatFromOpenGLFormat(ktx1->glFormat, ktx1->glType);
	return vk_format;
}

u32 get_ktx_api_specific_format(ktxTexture* This)
{
	return 0;
}

void CResourceLoader::initialize(IVirtualFileSystemInterface* vfs)
{
	if (!vfs->readBson("/root/embed/rendering/shader_cache.shc", m_shaderCache))
		log_error("Failed to load shader cache. ")
}

bool CResourceLoader::loadTexture(const utl::vector<uint8_t>& data, EMimeType mime, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info)
{
	create_info.typeFlags = ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eTransferable;

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
		return loadSTBFromMemory(data, create_info, upload_info);
	case EMimeType::eKTX:
		return loadKTXFromMemory(data, create_info, upload_info);
	case EMimeType::eKTX2:
		return loadKTX2FromMemory(data, create_info, upload_info);
	case EMimeType::eDDS:
		return loadDDSFromMemory(data, create_info, upload_info);
	}

	return false;
}

bool CResourceLoader::loadShaderData(const FShaderUploadInfo& load_info, utl::vector<FShaderCreationBlockInfo>& shader_stages)
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

		auto& shader_code_info = shader_stages.emplace_back();
		shader_code_info.stage = utl::cast<u32>(shader_module.type);
		shader_code_info.code = &found_shader_code->second;
	}

	return true;
}

bool CResourceLoader::loadSTBFromMemory(const utl::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info)
{
	int width{ 0 }, height{ 0 }, channels{ 0 };
	auto rawdata = stbi_load_from_memory(data.data(), data.size(), &width, &height, &channels, STBI_rgb_alpha);

	create_info.setFixedSize2D(utl::cast<u32>(width), utl::cast<u32>(height));
	create_info.format = 37;
	auto data_size = utl::cast<u32>(width * height * 4ull);

	upload_info.data.resize(data_size);
	std::memcpy(upload_info.data.data(), rawdata, data_size);

	create_info.typeFlags |= ETextureCreateFlagBits::e2D;

	create_info.mipLevels = utl::cast<u32>(std::floor(std::log2((std::max)(width, height)))) + 1u;

	stbi_image_free(rawdata);

	return true;
}

bool CResourceLoader::loadKTXFromMemory(const utl::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info)
{
	ktxTexture* kTexture{ nullptr };
	auto ktxresult = ktxTexture_CreateFromMemory(data.data(), data.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &kTexture);
	if (ktxresult != KTX_SUCCESS)
		return false;

	if (kTexture->isArray)
		create_info.typeFlags |= ETextureCreateFlagBits::eArray;
	if (kTexture->isCubemap)
		create_info.typeFlags |= ETextureCreateFlagBits::eCubemap;
	
	create_info.setFixedSize3D(kTexture->baseWidth, kTexture->baseHeight, kTexture->baseDepth);
	
	if (kTexture->numDimensions == 1u)
		create_info.typeFlags |= ETextureCreateFlagBits::e1D;
	else if (kTexture->numDimensions == 2u)
		create_info.typeFlags |= ETextureCreateFlagBits::e2D;
	else if (kTexture->numDimensions == 3u)
		create_info.typeFlags |= ETextureCreateFlagBits::e3D;

	create_info.mipLevels = kTexture->numLevels;
	create_info.arrayLayers = kTexture->numLayers;

	upload_info.data.resize(kTexture->dataSize);
	std::memcpy(upload_info.data.data(), kTexture->pData, kTexture->dataSize);

	// TODO: get api format
	create_info.format = get_ktx_format(kTexture);

	auto num_layers = kTexture->isCubemap ? kTexture->numFaces : kTexture->numLayers;
	for (u32 layer = 0; layer < num_layers; layer++)
	{
		auto& mip_layer = upload_info.mipLayerOffsets.emplace_back();
		for (u32 level = 0u; level < kTexture->numLevels; level++)
		{
			auto& mip_offset = mip_layer.emplace_back();
			ktxresult = ktxTexture_GetImageOffset(kTexture, level, 0, layer, &mip_offset);
		}
	}

	ktxTexture_Destroy(kTexture);

	return true;
}

bool CResourceLoader::loadKTX2FromMemory(const utl::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info)
{
	ktxTexture2* kTexture{ nullptr };
	auto ktxresult = ktxTexture_CreateFromMemory(data.data(), data.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, (ktxTexture**)&kTexture);
	if (ktxresult != KTX_SUCCESS)
		return false;

	create_info.typeFlags |= ETextureCreateFlagBits::eTransferable | ETextureCreateFlagBits::eSamplable;

	if (kTexture->isArray)
		create_info.typeFlags |= ETextureCreateFlagBits::eArray;
	if (kTexture->isCubemap)
		create_info.typeFlags |= ETextureCreateFlagBits::eCubemap;

	create_info.setFixedSize3D(kTexture->baseWidth, kTexture->baseHeight, kTexture->baseDepth);

	if (kTexture->numDimensions == 1u)
		create_info.typeFlags |= ETextureCreateFlagBits::e1D;
	else if (kTexture->numDimensions == 2u)
		create_info.typeFlags |= ETextureCreateFlagBits::e2D;
	else if (kTexture->numDimensions == 3u)
		create_info.typeFlags |= ETextureCreateFlagBits::e3D;

	create_info.mipLevels = kTexture->numLevels;
	create_info.arrayLayers = kTexture->numLayers;

	if (ktxTexture2_NeedsTranscoding(kTexture))
		ktxresult = ktxTexture2_TranscodeBasis(kTexture, KTX_TTF_BC7_RGBA, 0);

	upload_info.data.resize(kTexture->dataSize);
	std::memcpy(upload_info.data.data(), kTexture->pData, kTexture->dataSize);

	create_info.format = kTexture->vkFormat;

	auto num_layers = kTexture->isCubemap ? kTexture->numFaces : kTexture->numLayers;
	for (u32 layer = 0; layer < num_layers; layer++)
	{
		auto& mip_layer = upload_info.mipLayerOffsets.emplace_back();
		for (u32 level = 0u; level < kTexture->numLevels; level++)
		{
			auto& mip_offset = mip_layer.emplace_back();
			ktxresult = ktxTexture_GetImageOffset((ktxTexture*)kTexture, level, 0, layer, &mip_offset);
		}
	}

	ktxTexture_Destroy((ktxTexture*)kTexture);

	return true;
}

bool CResourceLoader::loadDDSFromMemory(const utl::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info)
{
	return false;
}