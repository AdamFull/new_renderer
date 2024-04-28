#pragma once

#include <container/flags.h>

namespace engine
{
	namespace render
	{
		enum class EApiDataFormat
		{
			eUnknown,

			eD32SFLOAT_8UINT,
			eD24UNORM_8UINT,
			eD16UNORM_8UINT,
			eD32SFLOAT,
			eD16UNORM,

			eRGBA8_SRGB,

			eR8_UNORM,
			eRG8_UNORM,
			eRGB8_UNORM,
			eRGBA8_UNORM,

			eR16_UNORM,
			eRG16_UNORM,
			eRGB16_UNORM,
			eRGBA16_UNORM,

			eR32_UINT,
			eRG32_UINT,
			eRGB32_UINT,
			eRGBA32_UINT,

			eR16_SFLOAT,
			eRG16_SFLOAT,
			eRGB16_SFLOAT,
			eRGBA16_SFLOAT,

			eR32_SFLOAT,
			eRG32_SFLOAT,
			eRGB32_SFLOAT,
			eRGBA32_SFLOAT,

			// Special formats
			eR11G11B10_UFLOAT,
		};

		enum class EResourceFlagBits
		{
			eTexture = 1 << 0,
			eBuffer = 1 << 1,
			eShader = 1 << 2
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(EResourceFlags, EResourceFlagBits)

		enum class EQueueTypeFlagBits
		{
			ePresent = 1 << 0,
			eGraphics = 1 << 1,
			eCompute = 1 << 2,
			eTransfer = 1 << 3
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(EQueueTypeFlags, EQueueTypeFlagBits)

		enum class ERenderGeometryCullerType
		{
			eScene,
			eCascadedShadow,
			eOmniShadow,
			eSpotShadow,
		};
	}
}