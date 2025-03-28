#pragma once

#include "../util/utl_enum.hpp"
#include <string_view>

namespace edge::gfx
{
	enum class Format {
		eUndefined,
		eR4G4UnormPack8,
		eR4G4B4A4UnormPack16,
		eB4G4R4A4UnormPack16,
		eR5G6B5UnormPack16,
		eB5G6R5UnormPack16,
		eR5G5B5A1UnormPack16,
		eB5G5R5A1UnormPack16,
		eA1R5G5B5UnormPack16,
		eR8Unorm,
		eR8Snorm,
		eR8Uscaled,
		eR8Sscaled,
		eR8Uint,
		eR8Sint,
		eR8Srgb,
		eR8G8Unorm,
		eR8G8Snorm,
		eR8G8Uscaled,
		eR8G8Sscaled,
		eR8G8Uint,
		eR8G8Sint,
		eR8G8Srgb,
		eR8G8B8Unorm,
		eR8G8B8Snorm,
		eR8G8B8Uscaled,
		eR8G8B8Sscaled,
		eR8G8B8Uint,
		eR8G8B8Sint,
		eR8G8B8Srgb,
		eB8G8R8Unorm,
		eB8G8R8Snorm,
		eB8G8R8Uscaled,
		eB8G8R8Sscaled,
		eB8G8R8Uint,
		eB8G8R8Sint,
		eB8G8R8Srgb,
		eR8G8B8A8Unorm,
		eR8G8B8A8Snorm,
		eR8G8B8A8Uscaled,
		eR8G8B8A8Sscaled,
		eR8G8B8A8Uint,
		eR8G8B8A8Sint,
		eR8G8B8A8Srgb,
		eB8G8R8A8Unorm,
		eB8G8R8A8Snorm,
		eB8G8R8A8Uscaled,
		eB8G8R8A8Sscaled,
		eB8G8R8A8Uint,
		eB8G8R8A8Sint,
		eB8G8R8A8Srgb,
		eA8B8G8R8UnormPack32,
		eA8B8G8R8SnormPack32,
		eA8B8G8R8UscaledPack32,
		eA8B8G8R8SscaledPack32,
		eA8B8G8R8UintPack32,
		eA8B8G8R8SintPack32,
		eA8B8G8R8SrgbPack32,
		eA2R10G10B10UnormPack32,
		eA2R10G10B10SnormPack32,
		eA2R10G10B10UscaledPack32,
		eA2R10G10B10SscaledPack32,
		eA2R10G10B10UintPack32,
		eA2R10G10B10SintPack32,
		eA2B10G10R10UnormPack32,
		eA2B10G10R10SnormPack32,
		eA2B10G10R10UscaledPack32,
		eA2B10G10R10SscaledPack32,
		eA2B10G10R10UintPack32,
		eA2B10G10R10SintPack32,
		eR16Unorm,
		eR16Snorm,
		eR16Uscaled,
		eR16Sscaled,
		eR16Uint,
		eR16Sint,
		eR16Sfloat,
		eR16G16Unorm,
		eR16G16Snorm,
		eR16G16Uscaled,
		eR16G16Sscaled,
		eR16G16Uint,
		eR16G16Sint,
		eR16G16Sfloat,
		eR16G16B16Unorm,
		eR16G16B16Snorm,
		eR16G16B16Uscaled,
		eR16G16B16Sscaled,
		eR16G16B16Uint,
		eR16G16B16Sint,
		eR16G16B16Sfloat,
		eR16G16B16A16Unorm,
		eR16G16B16A16Snorm,
		eR16G16B16A16Uscaled,
		eR16G16B16A16Sscaled,
		eR16G16B16A16Uint,
		eR16G16B16A16Sint,
		eR16G16B16A16Sfloat,
		eR32Uint,
		eR32Sint,
		eR32Sfloat,
		eR32G32Uint,
		eR32G32Sint,
		eR32G32Sfloat,
		eR32G32B32Uint,
		eR32G32B32Sint,
		eR32G32B32Sfloat,
		eR32G32B32A32Uint,
		eR32G32B32A32Sint,
		eR32G32B32A32Sfloat,
		eR64Uint,
		eR64Sint,
		eR64Sfloat,
		eR64G64Uint,
		eR64G64Sint,
		eR64G64Sfloat,
		eR64G64B64Uint,
		eR64G64B64Sint,
		eR64G64B64Sfloat,
		eR64G64B64A64Uint,
		eR64G64B64A64Sint,
		eR64G64B64A64Sfloat,
		eB10G11R11UfloatPack32,
		eE5B9G9R9UfloatPack32,
		eD16Unorm,
		eX8D24UnormPack32,
		eD32Sfloat,
		eS8Uint,
		eD16UnormS8Uint,
		eD24UnormS8Uint,
		eD32SfloatS8Uint,
		eBc1RgbUnormBlock,
		eBc1RgbSrgbBlock,
		eBc1RgbaUnormBlock,
		eBc1RgbaSrgbBlock,
		eBc2UnormBlock,
		eBc2SrgbBlock,
		eBc3UnormBlock,
		eBc3SrgbBlock,
		eBc4UnormBlock,
		eBc4SnormBlock,
		eBc5UnormBlock,
		eBc5SnormBlock,
		eBc6HUfloatBlock,
		eBc6HSfloatBlock,
		eBc7UnormBlock,
		eBc7SrgbBlock,
		eEtc2R8G8B8UnormBlock,
		eEtc2R8G8B8SrgbBlock,
		eEtc2R8G8B8A1UnormBlock,
		eEtc2R8G8B8A1SrgbBlock,
		eEtc2R8G8B8A8UnormBlock,
		eEtc2R8G8B8A8SrgbBlock,
		eEacR11UnormBlock,
		eEacR11SnormBlock,
		eEacR11G11UnormBlock,
		eEacR11G11SnormBlock,
		eAstc4x4UnormBlock,
		eAstc4x4SrgbBlock,
		eAstc5x4UnormBlock,
		eAstc5x4SrgbBlock,
		eAstc5x5UnormBlock,
		eAstc5x5SrgbBlock,
		eAstc6x5UnormBlock,
		eAstc6x5SrgbBlock,
		eAstc6x6UnormBlock,
		eAstc6x6SrgbBlock,
		eAstc8x5UnormBlock,
		eAstc8x5SrgbBlock,
		eAstc8x6UnormBlock,
		eAstc8x6SrgbBlock,
		eAstc8x8UnormBlock,
		eAstc8x8SrgbBlock,
		eAstc10x5UnormBlock,
		eAstc10x5SrgbBlock,
		eAstc10x6UnormBlock,
		eAstc10x6SrgbBlock,
		eAstc10x8UnormBlock,
		eAstc10x8SrgbBlock,
		eAstc10x10UnormBlock,
		eAstc10x10SrgbBlock,
		eAstc12x10UnormBlock,
		eAstc12x10SrgbBlock,
		eAstc12x12UnormBlock,
		eAstc12x12SrgbBlock,
		eG8B8G8R8422Unorm,
		eB8G8R8G8422Unorm,
		eG8B8R83Plane420Unorm,
		eG8B8R82Plane420Unorm,
		eG8B8R83Plane422UnormM,
		eG8B8R82Plane422Unorm,
		eG8B8R83Plane444Unorm,
		eR10X6UnormPack16,
		eR10X6G10X6Unorm2Pack16,
		eR10X6G10X6B10X6A10X6Unorm4Pack16,
		eG10X6B10X6G10X6R10X6422Unorm4Pack16,
		eB10X6G10X6R10X6G10X6422Unorm4Pack16,
		eG10X6B10X6R10X63Plane420Unorm3Pack16,
		eG10X6B10X6R10X62Plane420Unorm3Pack16,
		eG10X6B10X6R10X63Plane422Unorm3Pack16,
		eG10X6B10X6R10X62Plane422Unorm3Pack16,
		eG10X6B10X6R10X63Plane444Unorm3Pack16,
		eR12X4UnormPack16,
		eR12X4G12X4Unorm2Pack16,
		eR12X4G12X4B12X4A12X4Unorm4Pack16,
		eG12X4B12X4G12X4R12X4422Unorm4Pack16,
		eB12X4G12X4R12X4G12X4422Unorm4Pack16,
		eG12X4B12X4R12X43Plane420Unorm3Pack16,
		eG12X4B12X4R12X42Plane420Unorm3Pack16,
		eG12X4B12X4R12X43Plane422Unorm3Pack16,
		eG12X4B12X4R12X42Plane422Unorm3Pack16,
		eG12X4B12X4R12X43Plane444Unorm3Pack16,
		eG16B16G16R16422Unorm,
		eB16G16R16G16422Unorm,
		eG16B16R163Plane420Unorm,
		eG16B16R162Plane420Unorm,
		eG16B16R163Plane422Unorm,
		eG16B16R162Plane422Unorm,
		eG16B16R163Plane444Unorm,
		eG8B8R82Plane444Unorm,
		eG10X6B10X6R10X62Plane444Unorm3Pack16,
		eG12X4B12X4R12X42Plane444Unorm3Pack16,
		eG16B16R162Plane444Unorm,
		eA4R4G4B4UnormPack16,
		eA4B4G4R4UnormPack16,
		eAstc4x4SfloatBlock,
		eAstc5x4SfloatBlock,
		eAstc5x5SfloatBlock,
		eAstc6x5SfloatBlock,
		eAstc6x6SfloatBlock,
		eAstc8x5SfloatBlock,
		eAstc8x6SfloatBlock,
		eAstc8x8SfloatBlock,
		eAstc10x5SfloatBlock,
		eAstc10x6SfloatBlock,
		eAstc10x8SfloatBlock,
		eAstc10x10SfloatBlock,
		eAstc12x10SfloatBlock,
		eAstc12x12SfloatBlock
	};

	uint32_t format_to_vk(Format format);
	Format format_from_vk(uint32_t format);

	Format format_from_gl_internal(uint32_t format);
	Format format_from_gl(uint32_t format);

	inline bool is_depth_only_format(Format format) {
		return format == Format::eD16Unorm ||
			format == Format::eD32Sfloat;
	}

	inline bool is_depth_stencil_format(Format format) {
		return format == Format::eD16UnormS8Uint ||
			format == Format::eD24UnormS8Uint ||
			format == Format::eD32SfloatS8Uint;
	}

	inline bool is_depth_format(Format format) {
		return is_depth_only_format(format) || is_depth_stencil_format(format);
	}

	inline bool IsStencilFormat(Format format) {
		return !is_depth_only_format(format) || !is_depth_stencil_format(format);
	}

	inline uint64_t get_format_block_align(Format format) {
		switch (format) {
			// 8-bit packed formats
		case Format::eR4G4UnormPack8:
		case Format::eR8Unorm:
		case Format::eR8Snorm:
		case Format::eR8Uint:
		case Format::eR8Sint:
		case Format::eS8Uint:
			return 1ull;

			// 16-bit packed formats
		case Format::eR4G4B4A4UnormPack16:
		case Format::eB4G4R4A4UnormPack16:
		case Format::eR5G6B5UnormPack16:
		case Format::eB5G6R5UnormPack16:
		case Format::eR5G5B5A1UnormPack16:
		case Format::eB5G5R5A1UnormPack16:
		case Format::eA1R5G5B5UnormPack16:
		case Format::eR8G8Unorm:
		case Format::eR8G8Snorm:
		case Format::eR8G8Uint:
		case Format::eR8G8Sint:
		case Format::eR16Unorm:
		case Format::eR16Snorm:
		case Format::eR16Uint:
		case Format::eR16Sint:
		case Format::eR16Sfloat:
		case Format::eD16Unorm:
			return 2;

			// 24/32-bit packed formats
		case Format::eR8G8B8Unorm:
		case Format::eR8G8B8Snorm:
		case Format::eR8G8B8Uint:
		case Format::eR8G8B8Sint:
		case Format::eB8G8R8Unorm:
		case Format::eB8G8R8Snorm:
		case Format::eB8G8R8Uint:
		case Format::eB8G8R8Sint:
			return 3;

		case Format::eR8G8B8A8Unorm:
		case Format::eR8G8B8A8Snorm:
		case Format::eR8G8B8A8Uint:
		case Format::eR8G8B8A8Sint:
		case Format::eB8G8R8A8Unorm:
		case Format::eB8G8R8A8Snorm:
		case Format::eB8G8R8A8Uint:
		case Format::eB8G8R8A8Sint:
		case Format::eA8B8G8R8UnormPack32:
		case Format::eA8B8G8R8SnormPack32:
		case Format::eR32Uint:
		case Format::eR32Sint:
		case Format::eR32Sfloat:
		case Format::eB10G11R11UfloatPack32:
		case Format::eE5B9G9R9UfloatPack32:
		case Format::eD32Sfloat:
		case Format::eX8D24UnormPack32:
		case Format::eD24UnormS8Uint:
		case Format::eD32SfloatS8Uint:
			return 4;

			// 64-bit formats
		case Format::eR16G16Unorm:
		case Format::eR16G16Snorm:
		case Format::eR16G16Uint:
		case Format::eR16G16Sint:
		case Format::eR16G16Sfloat:
		case Format::eR64Uint:
		case Format::eR64Sint:
		case Format::eR64Sfloat:
		case Format::eR32G32Uint:
		case Format::eR32G32Sint:
		case Format::eR32G32Sfloat:
			return 8;

			// 128-bit formats
		case Format::eR32G32B32A32Uint:
		case Format::eR32G32B32A32Sint:
		case Format::eR32G32B32A32Sfloat:
		case Format::eR64G64Uint:
		case Format::eR64G64Sint:
		case Format::eR64G64Sfloat:
			return 16;

			// Compressed BC Formats
		case Format::eBc1RgbUnormBlock:
		case Format::eBc1RgbaUnormBlock:
		case Format::eBc1RgbSrgbBlock:
		case Format::eBc1RgbaSrgbBlock:
		case Format::eBc4UnormBlock:
		case Format::eBc4SnormBlock:
			return 8;

		case Format::eBc2UnormBlock:
		case Format::eBc2SrgbBlock:
		case Format::eBc3UnormBlock:
		case Format::eBc3SrgbBlock:
		case Format::eBc5UnormBlock:
		case Format::eBc5SnormBlock:
		case Format::eBc7SrgbBlock:
		case Format::eBc7UnormBlock:
		case Format::eBc6HUfloatBlock:
		case Format::eBc6HSfloatBlock:
			return 16;

			// ETC/EAC Formats
		case Format::eEtc2R8G8B8UnormBlock:
		case Format::eEtc2R8G8B8A8UnormBlock:
		case Format::eEacR11UnormBlock:
		case Format::eEacR11G11UnormBlock:
			return 16;

			// ASTC Formats (All ASTC blocks are 16-byte aligned)
		case Format::eAstc4x4UnormBlock:
		case Format::eAstc5x4UnormBlock:
		case Format::eAstc5x5UnormBlock:
		case Format::eAstc6x5UnormBlock:
		case Format::eAstc6x6UnormBlock:
		case Format::eAstc8x5UnormBlock:
		case Format::eAstc8x6UnormBlock:
		case Format::eAstc8x8UnormBlock:
		case Format::eAstc10x5UnormBlock:
		case Format::eAstc10x6UnormBlock:
		case Format::eAstc10x8UnormBlock:
		case Format::eAstc10x10UnormBlock:
		case Format::eAstc12x10UnormBlock:
		case Format::eAstc12x12UnormBlock:
			return 16;

		default:
			return 4;
		}
	}
}

EDGE_REGISTER_ENUM_CLASS(edge::gfx, Format,
	EDGE_ENUM_META(edge::gfx::Format::eUndefined, "undefined"),
	EDGE_ENUM_META(edge::gfx::Format::eR4G4UnormPack8, "r4g4_unorm_pack8"),
	EDGE_ENUM_META(edge::gfx::Format::eR4G4B4A4UnormPack16, "r4g4b4a4_unorm_pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eB4G4R4A4UnormPack16, "b4g4r4a4_unorm_pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eR5G6B5UnormPack16, "r5g6b5_unorm_pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eB5G6R5UnormPack16, "b5g6r5_unorm_pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eR5G5B5A1UnormPack16, "r5g5b5a1_unorm_pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eB5G5R5A1UnormPack16, "b5g5r5a1_unorm_pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eA1R5G5B5UnormPack16, "a1r5g5b5_unorm_pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eR8Unorm, "r8_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eR8Snorm, "r8_snorm"),
	EDGE_ENUM_META(edge::gfx::Format::eR8Uscaled, "r8_uscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eR8Sscaled, "r8_sscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eR8Uint, "r8_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eR8Sint, "r8_sint"),
	EDGE_ENUM_META(edge::gfx::Format::eR8Srgb, "r8_srgb"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8Unorm, "r8g8_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8Snorm, "r8g8_snorm"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8Uscaled, "r8g8_uscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8Sscaled, "r8g8_sscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8Uint, "r8g8_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8Sint, "r8g8_sint"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8Srgb, "r8g8_srgb"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8B8Unorm, "r8g8b8_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8B8Snorm, "r8g8b8_snorm"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8B8Uscaled, "r8g8b8_uscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8B8Sscaled, "r8g8b8_sscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8B8Uint, "r8g8b8_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8B8Sint, "r8g8b8_sint"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8B8Srgb, "r8g8b8_srgb"),
	EDGE_ENUM_META(edge::gfx::Format::eB8G8R8Unorm, "b8g8r8_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eB8G8R8Snorm, "b8g8r8_snorm"),
	EDGE_ENUM_META(edge::gfx::Format::eB8G8R8Uscaled, "b8g8r8_uscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eB8G8R8Sscaled, "b8g8r8_sscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eB8G8R8Uint, "b8g8r8_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eB8G8R8Sint, "b8g8r8_sint"),
	EDGE_ENUM_META(edge::gfx::Format::eB8G8R8Srgb, "b8g8r8_srgb"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8B8A8Unorm, "r8g8b8a8_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8B8A8Snorm, "r8g8b8a8_snorm"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8B8A8Uscaled, "r8g8b8a8_uscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8B8A8Sscaled, "r8g8b8a8_sscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8B8A8Uint, "r8g8b8a8_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8B8A8Sint, "r8g8b8a8_sint"),
	EDGE_ENUM_META(edge::gfx::Format::eR8G8B8A8Srgb, "r8g8b8a8_srgb"),
	EDGE_ENUM_META(edge::gfx::Format::eB8G8R8A8Unorm, "b8g8r8a8_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eB8G8R8A8Snorm, "b8g8r8a8_snorm"),
	EDGE_ENUM_META(edge::gfx::Format::eB8G8R8A8Uscaled, "b8g8r8a8_uscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eB8G8R8A8Sscaled, "b8g8r8a8_sscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eB8G8R8A8Uint, "b8g8r8a8_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eB8G8R8A8Sint, "b8g8r8a8_sint"),
	EDGE_ENUM_META(edge::gfx::Format::eB8G8R8A8Srgb, "b8g8r8a8_srgb"),
	EDGE_ENUM_META(edge::gfx::Format::eA8B8G8R8UnormPack32, "a8b8g8r8_unorm_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eA8B8G8R8SnormPack32, "a8b8g8r8_snorm_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eA8B8G8R8UscaledPack32, "a8b8g8r8_uscaled_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eA8B8G8R8SscaledPack32, "a8b8g8r8_sscaled_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eA8B8G8R8UintPack32, "a8b8g8r8_uint_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eA8B8G8R8SintPack32, "a8b8g8r8_sint_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eA8B8G8R8SrgbPack32, "a8b8g8r8_srgb_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eA2R10G10B10UnormPack32, "a2r10g10b10_unorm_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eA2R10G10B10SnormPack32, "a2r10g10b10_snorm_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eA2R10G10B10UscaledPack32, "a2r10g10b10_uscaled_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eA2R10G10B10SscaledPack32, "a2r10g10b10_sscaled_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eA2R10G10B10UintPack32, "a2r10g10b10_uint_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eA2R10G10B10SintPack32, "a2r10g10b10_sint_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eA2B10G10R10UnormPack32, "a2b10g10r10_unorm_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eA2B10G10R10SnormPack32, "a2b10g10r10_snorm_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eA2B10G10R10UscaledPack32, "a2b10g10r10_uscaled_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eA2B10G10R10SscaledPack32, "a2b10g10r10_sscaled_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eA2B10G10R10UintPack32, "a2b10g10r10_uint_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eA2B10G10R10SintPack32, "a2b10g10r10_sint_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eR16Unorm, "r16_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eR16Snorm, "r16_snorm"),
	EDGE_ENUM_META(edge::gfx::Format::eR16Uscaled, "r16_uscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eR16Sscaled, "r16_sscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eR16Uint, "r16_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eR16Sint, "r16_sint"),
	EDGE_ENUM_META(edge::gfx::Format::eR16Sfloat, "r16_sfloat"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16Unorm, "r16g16_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16Snorm, "r16g16_snorm"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16Uscaled, "r16g16_uscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16Sscaled, "r16g16_sscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16Uint, "r16g16_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16Sint, "r16g16_sint"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16Sfloat, "r16g16_sfloat"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16B16Unorm, "r16g16b16_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16B16Snorm, "r16g16b16_snorm"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16B16Uscaled, "r16g16b16_uscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16B16Sscaled, "r16g16b16_sscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16B16Uint, "r16g16b16_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16B16Sint, "r16g16b16_sint"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16B16Sfloat, "r16g16b16_sfloat"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16B16A16Unorm, "r16g16b16a16_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16B16A16Snorm, "r16g16b16a16_snorm"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16B16A16Uscaled, "r16g16b16a16_uscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16B16A16Sscaled, "r16g16b16a16_sscaled"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16B16A16Uint, "r16g16b16a16_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16B16A16Sint, "r16g16b16a16_sint"),
	EDGE_ENUM_META(edge::gfx::Format::eR16G16B16A16Sfloat, "r16g16b16a16_sfloat"),
	EDGE_ENUM_META(edge::gfx::Format::eR32Uint, "r32_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eR32Sint, "r32_sint"),
	EDGE_ENUM_META(edge::gfx::Format::eR32Sfloat, "r32_sfloat"),
	EDGE_ENUM_META(edge::gfx::Format::eR32G32Uint, "r32g32_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eR32G32Sint, "r32g32_sint"),
	EDGE_ENUM_META(edge::gfx::Format::eR32G32Sfloat, "r32g32_sfloat"),
	EDGE_ENUM_META(edge::gfx::Format::eR32G32B32Uint, "r32g32b32_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eR32G32B32Sint, "r32g32b32_sint"),
	EDGE_ENUM_META(edge::gfx::Format::eR32G32B32Sfloat, "r32g32b32_sfloat"),
	EDGE_ENUM_META(edge::gfx::Format::eR32G32B32A32Uint, "r32g32b32a32_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eR32G32B32A32Sint, "r32g32b32a32_sint"),
	EDGE_ENUM_META(edge::gfx::Format::eR32G32B32A32Sfloat, "r32g32b32a32_sfloat"),
	EDGE_ENUM_META(edge::gfx::Format::eR64Uint, "r64_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eR64Sint, "r64_sint"),
	EDGE_ENUM_META(edge::gfx::Format::eR64Sfloat, "r64_sfloat"),
	EDGE_ENUM_META(edge::gfx::Format::eR64G64Uint, "r64g64_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eR64G64Sint, "r64g64_sint"),
	EDGE_ENUM_META(edge::gfx::Format::eR64G64Sfloat, "r64g64_sfloat"),
	EDGE_ENUM_META(edge::gfx::Format::eR64G64B64Uint, "r64g64b64_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eR64G64B64Sint, "r64g64b64_sint"),
	EDGE_ENUM_META(edge::gfx::Format::eR64G64B64Sfloat, "r64g64b64_sfloat"),
	EDGE_ENUM_META(edge::gfx::Format::eR64G64B64A64Uint, "r64g64b64a64_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eR64G64B64A64Sint, "r64g64b64a64_sint"),
	EDGE_ENUM_META(edge::gfx::Format::eR64G64B64A64Sfloat, "r64g64b64a64_sfloat"),
	EDGE_ENUM_META(edge::gfx::Format::eB10G11R11UfloatPack32, "b10g11r11_ufloat_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eE5B9G9R9UfloatPack32, "e5b9g9r9_ufloat_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eD16Unorm, "d16_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eX8D24UnormPack32, "x8_d24_unorm_pack32"),
	EDGE_ENUM_META(edge::gfx::Format::eD32Sfloat, "d32_sfloat"),
	EDGE_ENUM_META(edge::gfx::Format::eS8Uint, "s8_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eD16UnormS8Uint, "d16_unorm_s8_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eD24UnormS8Uint, "d24_unorm_s8_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eD32SfloatS8Uint, "d32_sfloat_s8_uint"),
	EDGE_ENUM_META(edge::gfx::Format::eBc1RgbUnormBlock, "bc1_rgb_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eBc1RgbSrgbBlock, "bc1_rgb_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eBc1RgbaUnormBlock, "bc1_rgba_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eBc1RgbaSrgbBlock, "bc1_rgba_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eBc2UnormBlock, "bc2_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eBc2SrgbBlock, "bc2_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eBc3UnormBlock, "bc3_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eBc3SrgbBlock, "bc3_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eBc4UnormBlock, "bc4_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eBc4SnormBlock, "bc4_snorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eBc5UnormBlock, "bc5_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eBc5SnormBlock, "bc5_snorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eBc6HUfloatBlock, "bc6h_ufloat_block"),
	EDGE_ENUM_META(edge::gfx::Format::eBc6HSfloatBlock, "bc6h_sfloat_block"),
	EDGE_ENUM_META(edge::gfx::Format::eBc7UnormBlock, "bc7_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eBc7SrgbBlock, "bc7_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eEtc2R8G8B8UnormBlock, "etc2_r8g8b8_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eEtc2R8G8B8SrgbBlock, "etc2_r8g8b8_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eEtc2R8G8B8A1UnormBlock, "etc2_r8g8b8a1_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eEtc2R8G8B8A1SrgbBlock, "etc2_r8g8b8a1_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eEtc2R8G8B8A8UnormBlock, "etc2_r8g8b8a8_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eEtc2R8G8B8A8SrgbBlock, "etc2_r8g8b8a8_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eEacR11UnormBlock, "eac_r11_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eEacR11SnormBlock, "eac_r11_snorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eEacR11G11UnormBlock, "eac_r11g11_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eEacR11G11SnormBlock, "eac_r11g11_snorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc4x4UnormBlock, "astc_4x4_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc4x4SrgbBlock, "astc_4x4_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc5x4UnormBlock, "astc_5x4_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc5x4SrgbBlock, "astc_5x4_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc5x5UnormBlock, "astc_5x5_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc5x5SrgbBlock, "astc_5x5_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc6x5UnormBlock, "astc_6x5_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc6x5SrgbBlock, "astc_6x5_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc6x6UnormBlock, "astc_6x6_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc6x6SrgbBlock, "astc_6x6_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc8x5UnormBlock, "astc_8x5_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc8x5SrgbBlock, "astc_8x5_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc8x6UnormBlock, "astc_8x6_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc8x6SrgbBlock, "astc_8x6_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc8x8UnormBlock, "astc_8x8_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc8x8SrgbBlock, "astc_8x8_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc10x5UnormBlock, "astc_10x5_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc10x5SrgbBlock, "astc_10x5_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc10x6UnormBlock, "astc_10x6_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc10x6SrgbBlock, "astc_10x6_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc10x8UnormBlock, "astc_10x8_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc10x8SrgbBlock, "astc_10x8_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc10x10UnormBlock, "astc_10x10_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc10x10SrgbBlock, "astc_10x10_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc12x10UnormBlock, "astc_12x10_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc12x10SrgbBlock, "astc_12x10_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc12x12UnormBlock, "astc_12x12_unorm_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc12x12SrgbBlock, "astc_12x12_srgb_block"),
	EDGE_ENUM_META(edge::gfx::Format::eG8B8G8R8422Unorm, "g8b8g8r8_422_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eB8G8R8G8422Unorm, "b8g8r8g8_422_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eG8B8R83Plane420Unorm, "g8_b8_r8_3plane_420_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eG8B8R82Plane420Unorm, "g8_b8r8_2plane_420_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eG8B8R83Plane422UnormM, "g8_b8_r8_3plane_422_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eG8B8R82Plane422Unorm, "g8_b8r8_2plane_422_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eG8B8R83Plane444Unorm, "g8_b8_r8_3plane_444_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eR10X6UnormPack16, "r10x6_unorm_pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eR10X6G10X6Unorm2Pack16, "r10x6g10x6_unorm_2pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eR10X6G10X6B10X6A10X6Unorm4Pack16, "r10x6g10x6b10x6a10x6_unorm_4pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eG10X6B10X6G10X6R10X6422Unorm4Pack16, "g10x6b10x6g10x6r10x6_422_unorm_4pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eB10X6G10X6R10X6G10X6422Unorm4Pack16, "b10x6g10x6r10x6g10x6_422_unorm_4pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eG10X6B10X6R10X63Plane420Unorm3Pack16, "g10x6_b10x6_r10x6_3plane_420_unorm_3pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eG10X6B10X6R10X62Plane420Unorm3Pack16, "g10x6_b10x6r10x6_2plane_420_unorm_3pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eG10X6B10X6R10X63Plane422Unorm3Pack16, "g10x6_b10x6_r10x6_3plane_422_unorm_3pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eG10X6B10X6R10X62Plane422Unorm3Pack16, "g10x6_b10x6r10x6_2plane_422_unorm_3pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eG10X6B10X6R10X63Plane444Unorm3Pack16, "g10x6_b10x6_r10x6_3plane_444_unorm_3pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eR12X4UnormPack16, "r12x4_unorm_pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eR12X4G12X4Unorm2Pack16, "r12x4g12x4_unorm_2pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eR12X4G12X4B12X4A12X4Unorm4Pack16, "r12x4g12x4b12x4a12x4_unorm_4pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eG12X4B12X4G12X4R12X4422Unorm4Pack16, "g12x4b12x4g12x4r12x4_422_unorm_4pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eB12X4G12X4R12X4G12X4422Unorm4Pack16, "b12x4g12x4r12x4g12x4_422_unorm_4pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eG12X4B12X4R12X43Plane420Unorm3Pack16, "g12x4_b12x4_r12x4_3plane_420_unorm_3pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eG12X4B12X4R12X42Plane420Unorm3Pack16, "g12x4_b12x4r12x4_2plane_420_unorm_3pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eG12X4B12X4R12X43Plane422Unorm3Pack16, "g12x4_b12x4_r12x4_3plane_422_unorm_3pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eG12X4B12X4R12X42Plane422Unorm3Pack16, "g12x4_b12x4r12x4_2plane_422_unorm_3pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eG12X4B12X4R12X43Plane444Unorm3Pack16, "g12x4_b12x4_r12x4_3plane_444_unorm_3pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eG16B16G16R16422Unorm, "g16b16g16r16_422_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eB16G16R16G16422Unorm, "b16g16r16g16_422_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eG16B16R163Plane420Unorm, "g16_b16_r16_3plane_420_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eG16B16R162Plane420Unorm, "g16_b16r16_2plane_420_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eG16B16R163Plane422Unorm, "g16_b16_r16_3plane_422_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eG16B16R162Plane422Unorm, "g16_b16r16_2plane_422_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eG16B16R163Plane444Unorm, "g16_b16_r16_3plane_444_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eG8B8R82Plane444Unorm, "g8_b8r8_2plane_444_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eG10X6B10X6R10X62Plane444Unorm3Pack16, "g10x6_b10x6r10x6_2plane_444_unorm_3pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eG12X4B12X4R12X42Plane444Unorm3Pack16, "g12x4_b12x4r12x4_2plane_444_unorm_3pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eG16B16R162Plane444Unorm, "g16_b16r16_2plane_444_unorm"),
	EDGE_ENUM_META(edge::gfx::Format::eA4R4G4B4UnormPack16, "a4r4g4b4_unorm_pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eA4B4G4R4UnormPack16, "a4b4g4r4_unorm_pack16"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc4x4SfloatBlock, "astc_4x4_sfloat_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc5x4SfloatBlock, "astc_5x4_sfloat_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc5x5SfloatBlock, "astc_5x5_sfloat_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc6x5SfloatBlock, "astc_6x5_sfloat_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc6x6SfloatBlock, "astc_6x6_sfloat_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc8x5SfloatBlock, "astc_8x5_sfloat_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc8x6SfloatBlock, "astc_8x6_sfloat_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc8x8SfloatBlock, "astc_8x8_sfloat_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc10x5SfloatBlock, "astc_10x5_sfloat_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc10x6SfloatBlock, "astc_10x6_sfloat_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc10x8SfloatBlock, "astc_10x8_sfloat_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc10x10SfloatBlock, "astc_10x10_sfloat_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc12x10SfloatBlock, "astc_12x10_sfloat_block"),
	EDGE_ENUM_META(edge::gfx::Format::eAstc12x12SfloatBlock, "astc_12x12_sfloat_block")
)