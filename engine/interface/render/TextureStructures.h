#pragma once

#include <vector>

namespace engine
{
	namespace render
	{
		enum ETextureImageTypeFlagBits
		{
			TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT = 1 << 0,
			TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT = 1 << 1,
			TEXTURE_IMAGE_TYPE_1D_FLAG_BIT = 1 << 2,
			TEXTURE_IMAGE_TYPE_2D_FLAG_BIT = 1 << 3,
			TEXTURE_IMAGE_TYPE_3D_FLAG_BIT = 1 << 4,
			TEXTURE_IMAGE_TYPE_DEPTH_FLAG_BIT = 1 << 5,
			TEXTURE_IMAGE_TYPE_TRANSFERABLE_FLAG_BIT = 1 << 6,
			TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT = 1 << 7,
			TEXTURE_IMAGE_TYPE_STORAGE_FLAG_BIT = 1 << 8
		};

		struct FTextureCreateInfo
		{
			bool isSwapchainTarget{ false };

			// EApiDataFormat
			uint32_t format{ 0u };
			uint32_t typeFlags{ 0u };

			uint32_t width{ 1u };
			uint32_t height{ 1u };
			uint32_t depth{ 1u };

			uint32_t mipLevels{ 1u };
			uint32_t arrayLayers{ 1u };
		};

		struct FTextureUploadInfo
		{
			size_t textureId{ ~0ull };
			std::vector<uint8_t> data{};
			std::vector<std::vector<size_t>> mipLayerOffsets{};
		};
	}
}