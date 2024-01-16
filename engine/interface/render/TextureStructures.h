#pragma once

#include <utility.hpp>

namespace engine
{
	namespace render
	{
		enum class ETextureCreateFlagBits : uint32_t
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

		enum class ERenderTargetSizePolicy : uint32_t
		{
			eFull = 1,
			eScalable = 2,
			eFixed = 3
		};

		struct FTextureCreateInfo
		{
			bool isSwapchainTarget{ false };

			// EApiDataFormat
			uint32_t format{ 0u };
			ETextureCreateFlags typeFlags{ 0u };
			ERenderTargetSizePolicy sizePolicy{};

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