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
			eStorage = 1 << 7,
			eRenderTarget = 1 << 8
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
			ERenderTargetSizePolicy policy{ ERenderTargetSizePolicy::eScalable };

			u32 width{ 1u };
			u32 height{ 1u };
			u32 depth{ 1u };

			f32 scale{ 1.f };
		};

		struct FTextureCreateInfo
		{
			// EApiDataFormat
			EApiDataFormat format{ EApiDataFormat::eUnknown };
			ETextureCreateFlags flags{ 0u };
			FTextureSizeInfo size{};

			u32 mipLevels{ 1u };
			u32 arrayLayers{ 1u };

			void SetFixedSize3D(u32 n_width, u32 n_height, u32 n_depth)
			{
				size.width = n_width;
				size.height = n_height;
				size.depth = n_depth;

				size.policy = ERenderTargetSizePolicy::eFixed;
			}

			void SetFixedSize3D(u32 size) { SetFixedSize3D(size, size, size); }
			void SetFixedSize2D(u32 n_width, u32 n_height) { SetFixedSize3D(n_width, n_height, size.depth); }
			void SetFixedSize2D(u32 size) { SetFixedSize2D(size, size); }

			void SetSizeScale(f32 scale)
			{
				size.scale = scale;
				size.policy = ERenderTargetSizePolicy::eScalable;
			}

			void EnableMipMap() { mipLevels = utl::cast<u32>(std::floor(std::log2((std::max)(size.width, size.height)))) + 1u; }
		};

		struct FTextureUploadInfo
		{
			FAPIHandle textureId{ INVALID_HANDLE };
			utl::vector<uint8_t> data{};
			utl::vector<utl::vector<u64>> mipLayerOffsets{};
		};

		struct FSamplerCreateInfo
		{
			ESamplerFilterType filter{};
			ESamplerAddressMode addressMode{};
			u32 mipLevels{ 1u };
		};

		struct FTextureView
		{
			u32 width{ 1u }, height{ 1u }, depth{ 1u };
			u32 mipLevels{ 1u };
			u32 arrayLayers{ 1u };
		};

		class ITextureObject
		{
		public:
			virtual ~ITextureObject() = default;

			virtual void Create(const std::string& name, const FTextureCreateInfo& create_info) = 0;
			virtual void Create(const std::string& name, const FTextureCreateInfo& create_info, FTextureUploadInfo&& upload_info) = 0;

			virtual void GetDimensions(u32* width, u32* height = nullptr, u32* depth = nullptr) = 0;
			virtual void GetMipLevelCount(u32* mip_count) = 0;
			virtual void GetLayerCount(u32* layer_count) = 0;

			virtual FAPIHandle Handle() = 0;
		};
	}
}