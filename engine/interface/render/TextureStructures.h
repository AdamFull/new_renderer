#pragma once

#include <utility.hpp>

namespace engine
{
	namespace render
	{
		enum class ETextureCreateFlagBits : u32
		{
			eArray = 1 << 0,
			eCubemap = 1 << 1,
			e1D = 1 << 2,
			e2D = 1 << 3,
			e3D = 1 << 4,
			eTransferable = 1 << 5,
			eSamplable = 1 << 6,
			eStorage = 1 << 7
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(ETextureCreateFlags, ETextureCreateFlagBits)

		enum class ERenderTargetSizePolicy : u32
		{
			eFull = 1,
			eScalable = 2,
			eFixed = 3
		};

		struct FTextureCreateInfo
		{
			bool isSwapchainTarget{ false };

			// EApiDataFormat
			u32 format{ 0u };
			ETextureCreateFlags typeFlags{ 0u };
			ERenderTargetSizePolicy sizePolicy{};

			u32 width{ 1u };
			u32 height{ 1u };
			u32 depth{ 1u };

			u32 mipLevels{ 1u };
			u32 arrayLayers{ 1u };
		};

		struct FTextureUploadInfo
		{
			u64 textureId{ ~0ull };
			std::vector<uint8_t> data{};
			std::vector<std::vector<u64>> mipLayerOffsets{};
		};
	}
}