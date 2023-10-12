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

FTextureCreateInfo CResourceLoader::loadTexture(const std::vector<uint8_t>& data, EMimeType mime)
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
		return loadSTBFromMemory(data);
	case EMimeType::eKTX:
		return loadKTXFromMemory(data);
	case EMimeType::eKTX2:
		return loadKTX2FromMemory(data);
	case EMimeType::eDDS:
		return loadDDSFromMemory(data);
	}

	return FTextureCreateInfo{};
}

FTextureCreateInfo CResourceLoader::loadSTBFromMemory(const std::vector<uint8_t>& data)
{
	FTextureCreateInfo texture_ci{};

	int width{ 0 }, height{ 0 }, channels{ 0 };
	auto rawdata = stbi_load_from_memory(data.data(), data.size(), &width, &height, &channels, STBI_rgb_alpha);

	texture_ci.result = true;
	texture_ci.width = static_cast<uint32_t>(width);
	texture_ci.height = static_cast<uint32_t>(height);
	texture_ci.depth = 1u;
	texture_ci.format = 37;
	auto data_size = static_cast<uint32_t>(width * height * 4ull);

	texture_ci.data.resize(data_size);
	std::memcpy(texture_ci.data.data(), rawdata, data_size);

	texture_ci.typeFlags |= TEXTURE_IMAGE_TYPE_3D_FLAG_BIT;

	texture_ci.mipLevels = static_cast<uint32_t>(std::floor(std::log2((std::max)(width, height)))) + 1u;

	stbi_image_free(rawdata);

	return texture_ci;
}

FTextureCreateInfo CResourceLoader::loadKTXFromMemory(const std::vector<uint8_t>& data)
{
	FTextureCreateInfo texture_ci{};

	ktxTexture* kTexture{ nullptr };
	auto ktxresult = ktxTexture_CreateFromMemory(data.data(), data.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &kTexture);

	if (kTexture->isArray)
		texture_ci.typeFlags |= TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT;
	if (kTexture->isCubemap)
		texture_ci.typeFlags |= TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT;
	
	texture_ci.result = true;
	texture_ci.width = kTexture->baseWidth;
	texture_ci.height = kTexture->baseHeight;
	texture_ci.depth = kTexture->baseDepth;
	
	if (kTexture->numDimensions == 1u)
		texture_ci.typeFlags |= TEXTURE_IMAGE_TYPE_1D_FLAG_BIT;
	else if (kTexture->numDimensions == 2u)
		texture_ci.typeFlags |= TEXTURE_IMAGE_TYPE_2D_FLAG_BIT;
	else if (kTexture->numDimensions == 3u)
		texture_ci.typeFlags |= TEXTURE_IMAGE_TYPE_3D_FLAG_BIT;

	texture_ci.mipLevels = kTexture->numLevels;
	texture_ci.arrayLayers = kTexture->numLayers;

	texture_ci.data.resize(kTexture->dataSize);
	std::memcpy(texture_ci.data.data(), kTexture->pData, kTexture->dataSize);

	// TODO: get api format
	texture_ci.format = get_ktx_format(kTexture);

	auto num_layers = kTexture->isCubemap ? kTexture->numFaces : kTexture->numLayers;
	for (uint32_t layer = 0; layer < num_layers; layer++)
	{
		auto& mip_layer = texture_ci.offsets.emplace_back();
		for (uint32_t level = 0u; level < kTexture->numLevels; level++)
		{
			auto& mip_offset = mip_layer.emplace_back();
			ktxresult = ktxTexture_GetImageOffset(kTexture, level, 0, layer, &mip_offset);
		}
	}

	ktxTexture_Destroy(kTexture);

	return texture_ci;
}

FTextureCreateInfo CResourceLoader::loadKTX2FromMemory(const std::vector<uint8_t>& data)
{
	FTextureCreateInfo texture_ci{};

	ktxTexture2* kTexture{ nullptr };
	auto ktxresult = ktxTexture_CreateFromMemory(data.data(), data.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, (ktxTexture**)&kTexture);

	if (kTexture->isArray)
		texture_ci.typeFlags |= TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT;
	if (kTexture->isCubemap)
		texture_ci.typeFlags |= TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT;

	texture_ci.result = true;
	texture_ci.width = kTexture->baseWidth;
	texture_ci.height = kTexture->baseHeight;
	texture_ci.depth = kTexture->baseDepth;

	if (kTexture->numDimensions == 1u)
		texture_ci.typeFlags |= TEXTURE_IMAGE_TYPE_1D_FLAG_BIT;
	else if (kTexture->numDimensions == 2u)
		texture_ci.typeFlags |= TEXTURE_IMAGE_TYPE_2D_FLAG_BIT;
	else if (kTexture->numDimensions == 3u)
		texture_ci.typeFlags |= TEXTURE_IMAGE_TYPE_3D_FLAG_BIT;

	texture_ci.mipLevels = kTexture->numLevels;
	texture_ci.arrayLayers = kTexture->numLayers;

	if (ktxTexture2_NeedsTranscoding(kTexture))
		ktxresult = ktxTexture2_TranscodeBasis(kTexture, KTX_TTF_BC7_RGBA, 0);

	texture_ci.data.resize(kTexture->dataSize);
	std::memcpy(texture_ci.data.data(), kTexture->pData, kTexture->dataSize);

	texture_ci.format = kTexture->vkFormat;

	auto num_layers = kTexture->isCubemap ? kTexture->numFaces : kTexture->numLayers;
	for (uint32_t layer = 0; layer < num_layers; layer++)
	{
		auto& mip_layer = texture_ci.offsets.emplace_back();
		for (uint32_t level = 0u; level < kTexture->numLevels; level++)
		{
			auto& mip_offset = mip_layer.emplace_back();
			ktxresult = ktxTexture_GetImageOffset((ktxTexture*)kTexture, level, 0, layer, &mip_offset);
		}
	}

	ktxTexture_Destroy((ktxTexture*)kTexture);

	return texture_ci;
}

FTextureCreateInfo CResourceLoader::loadDDSFromMemory(const std::vector<uint8_t>& data)
{
	return FTextureCreateInfo{};
}