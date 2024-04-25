#pragma once

#include <standart_library.h>

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

		enum class ESamplerFilterType
		{
			eNearest,
			eLinear,
			eCubic
		};

		enum class ESamplerAddressMode
		{
			eRepeat,
			eMirroredRepeat,
			eClampToEdge,
			eClampToBorder,
			eMirrorClampToEdge
		};

		enum class ERenderTargetSizePolicy : u32
		{
			eFull = 1,
			eScalable = 2,
			eFixed = 3
		};

		struct FTextureSizeInfo
		{
			ERenderTargetSizePolicy policy{};

			u32 width{ 1u };
			u32 height{ 1u };
			u32 depth{ 1u };

			f32 scale{ 1.f };
		};

		struct FTextureCreateInfo
		{
			// EApiDataFormat
			u32 format{ 0u };
			ETextureCreateFlags typeFlags{ 0u };
			FTextureSizeInfo size{};

			u32 mipLevels{ 1u };
			u32 arrayLayers{ 1u };

			void setFixedSize3D(u32 n_width, u32 n_height, u32 n_depth)
			{
				size.width = n_width;
				size.height = n_height;
				size.depth = n_depth;

				size.policy = ERenderTargetSizePolicy::eFixed;
			}

			void setFixedSize3D(u32 size) { setFixedSize3D(size, size, size); }
			void setFixedSize2D(u32 n_width, u32 n_height) { setFixedSize3D(n_width, n_height, size.depth); }
			void setFixedSize2D(u32 size) { setFixedSize2D(size, size); }

			void setSizeScale(f32 scale)
			{
				size.scale = scale;
				size.policy = ERenderTargetSizePolicy::eScalable;
			}

			void enableMipMap() { mipLevels = utl::cast<u32>(std::floor(std::log2((std::max)(size.width, size.height)))) + 1u; }
		};

		struct FTextureUploadInfo
		{
			u64 textureId{ INVALID_HANDLE };
			utl::vector<uint8_t> data{};
			utl::vector<utl::vector<u64>> mipLayerOffsets{};
		};

		struct FSamplerCreateInfo
		{
			ESamplerFilterType filter{};
			ESamplerAddressMode addressMode{};
			u32 mipLevels{ 1u };
		};
	}
}