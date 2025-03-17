#pragma once

#include <cstdint>

namespace edge
{
    namespace gfx
    {
        enum class Format
        {
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

        uint32_t FormatToVk(Format format);
        Format FormatFromVk(uint32_t format);

        Format FormatFromGLInternal(uint32_t format);
        Format FormatFromGL(uint32_t format);

        inline bool IsDepthOnlyFormat(Format format)
        {
            return format == Format::eD16Unorm ||
                format == Format::eD32Sfloat;
        }

        inline bool IsDepthStencilFormat(Format format)
        {
            return format == Format::eD16UnormS8Uint ||
                format == Format::eD24UnormS8Uint ||
                format == Format::eD32SfloatS8Uint;
        }

        inline bool IsDepthFormat(Format format)
        {
            return IsDepthOnlyFormat(format) || IsDepthStencilFormat(format);
        }

        inline bool IsStencilFormat(Format format)
        {
            return !IsDepthOnlyFormat(format) || !IsDepthStencilFormat(format);
        }

        inline uint64_t GetFormatBlockAlign(Format format)
        {
            switch (format)
            {
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
}