#include "ResourceLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <ktx.h>
#include <vk_format.h>

using namespace engine;
using namespace engine::render;

uint32_t get_ktx_format(ktxTexture* This)
{
	if (This->classId == ktxTexture2_c)
		return ((ktxTexture2*)This)->vkFormat;

	auto* ktx1 = (ktxTexture1*)This;
	uint32_t vk_format{};
	vk_format = vkGetFormatFromOpenGLInternalFormat(ktx1->glInternalformat);
	if (vk_format == 0u)
		vk_format = vkGetFormatFromOpenGLFormat(ktx1->glFormat, ktx1->glType);
	return vk_format;
}

uint32_t get_ktx_api_specific_format(ktxTexture* This)
{
	return 0;
}

bool CResourceLoader::loadTexture(const std::vector<uint8_t>& data, EMimeType mime, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info)
{
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

bool CResourceLoader::loadSTBFromMemory(const std::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info)
{
	int width{ 0 }, height{ 0 }, channels{ 0 };
	auto rawdata = stbi_load_from_memory(data.data(), data.size(), &width, &height, &channels, STBI_rgb_alpha);

	create_info.width = static_cast<uint32_t>(width);
	create_info.height = static_cast<uint32_t>(height);
	create_info.depth = 1u;
	create_info.format = 37;
	auto data_size = static_cast<uint32_t>(width * height * 4ull);

	upload_info.data.resize(data_size);
	std::memcpy(upload_info.data.data(), rawdata, data_size);

	create_info.typeFlags |= TEXTURE_IMAGE_TYPE_3D_FLAG_BIT;

	create_info.mipLevels = static_cast<uint32_t>(std::floor(std::log2((std::max)(width, height)))) + 1u;

	stbi_image_free(rawdata);

	return true;
}

bool CResourceLoader::loadKTXFromMemory(const std::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info)
{
	ktxTexture* kTexture{ nullptr };
	auto ktxresult = ktxTexture_CreateFromMemory(data.data(), data.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &kTexture);
	if (ktxresult != KTX_SUCCESS)
		return false;

	if (kTexture->isArray)
		create_info.typeFlags |= TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT;
	if (kTexture->isCubemap)
		create_info.typeFlags |= TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT;
	
	create_info.width = kTexture->baseWidth;
	create_info.height = kTexture->baseHeight;
	create_info.depth = kTexture->baseDepth;
	
	if (kTexture->numDimensions == 1u)
		create_info.typeFlags |= TEXTURE_IMAGE_TYPE_1D_FLAG_BIT;
	else if (kTexture->numDimensions == 2u)
		create_info.typeFlags |= TEXTURE_IMAGE_TYPE_2D_FLAG_BIT;
	else if (kTexture->numDimensions == 3u)
		create_info.typeFlags |= TEXTURE_IMAGE_TYPE_3D_FLAG_BIT;

	create_info.mipLevels = kTexture->numLevels;
	create_info.arrayLayers = kTexture->numLayers;

	upload_info.data.resize(kTexture->dataSize);
	std::memcpy(upload_info.data.data(), kTexture->pData, kTexture->dataSize);

	// TODO: get api format
	create_info.format = get_ktx_format(kTexture);

	auto num_layers = kTexture->isCubemap ? kTexture->numFaces : kTexture->numLayers;
	for (uint32_t layer = 0; layer < num_layers; layer++)
	{
		auto& mip_layer = upload_info.mipLayerOffsets.emplace_back();
		for (uint32_t level = 0u; level < kTexture->numLevels; level++)
		{
			auto& mip_offset = mip_layer.emplace_back();
			ktxresult = ktxTexture_GetImageOffset(kTexture, level, 0, layer, &mip_offset);
		}
	}

	ktxTexture_Destroy(kTexture);

	return true;
}

bool CResourceLoader::loadKTX2FromMemory(const std::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info)
{
	ktxTexture2* kTexture{ nullptr };
	auto ktxresult = ktxTexture_CreateFromMemory(data.data(), data.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, (ktxTexture**)&kTexture);
	if (ktxresult != KTX_SUCCESS)
		return false;

	create_info.typeFlags |= TEXTURE_IMAGE_TYPE_TRANSFERABLE_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT;

	if (kTexture->isArray)
		create_info.typeFlags |= TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT;
	if (kTexture->isCubemap)
		create_info.typeFlags |= TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT;

	create_info.width = kTexture->baseWidth;
	create_info.height = kTexture->baseHeight;
	create_info.depth = kTexture->baseDepth;

	if (kTexture->numDimensions == 1u)
		create_info.typeFlags |= TEXTURE_IMAGE_TYPE_1D_FLAG_BIT;
	else if (kTexture->numDimensions == 2u)
		create_info.typeFlags |= TEXTURE_IMAGE_TYPE_2D_FLAG_BIT;
	else if (kTexture->numDimensions == 3u)
		create_info.typeFlags |= TEXTURE_IMAGE_TYPE_3D_FLAG_BIT;

	create_info.mipLevels = kTexture->numLevels;
	create_info.arrayLayers = kTexture->numLayers;

	if (ktxTexture2_NeedsTranscoding(kTexture))
		ktxresult = ktxTexture2_TranscodeBasis(kTexture, KTX_TTF_BC7_RGBA, 0);

	upload_info.data.resize(kTexture->dataSize);
	std::memcpy(upload_info.data.data(), kTexture->pData, kTexture->dataSize);

	create_info.format = kTexture->vkFormat;

	auto num_layers = kTexture->isCubemap ? kTexture->numFaces : kTexture->numLayers;
	for (uint32_t layer = 0; layer < num_layers; layer++)
	{
		auto& mip_layer = upload_info.mipLayerOffsets.emplace_back();
		for (uint32_t level = 0u; level < kTexture->numLevels; level++)
		{
			auto& mip_offset = mip_layer.emplace_back();
			ktxresult = ktxTexture_GetImageOffset((ktxTexture*)kTexture, level, 0, layer, &mip_offset);
		}
	}

	ktxTexture_Destroy((ktxTexture*)kTexture);

	return true;
}

bool CResourceLoader::loadDDSFromMemory(const std::vector<uint8_t>& data, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info)
{
	return false;
}