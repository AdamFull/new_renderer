#pragma once

#include "BaseEnumerators.h"
#include "BufferStructures.h"
#include <embed/shaders/includes/interop/base.h>

namespace engine
{
	namespace render
	{
		enum class EVertexAttributeType : u32
		{
			ePosition,
			eColor,
			eTexCoord,
			eNormal,
			eTangent,
			eJoint,
			eWeight,
			eIndex
		};

		enum class EGeometryBufferIndices
		{
			eVertexBuffer,
			eIndexBuffer,
			eMeshletBuffer,
			eVertexRemapBuffer,
			eBoundsBuffer,
			eTriangleBuffer,
			eLodBuffer,

			eLastIndex
		};

		enum class EVertexHandlerFlagBits
		{
			eGenerateMeshlets = 1 << 0,
			eGenerateLods = 1 << 1,
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(EVertexHandlerFlags, EVertexHandlerFlagBits)

		struct FVertexAttributePointer
		{
			EVertexAttributeType type{ EVertexAttributeType::eIndex };
			const uint8_t* data{ nullptr };
			u64 stride{ 0ull };
			u64 count{ 0ull };
		};

		struct FVertexBufferBindingInfo
		{
			u32 firstBinding{ 0u };
			utl::vector<u64> buffers{};
			utl::vector<u64> offsets{};
		};

		struct FIndexBufferBindingInfo
		{
			u64 bufferId{ INVALID_HANDLE };
			u64 offset{ 0ull };
			u64 indexSize{ 0ull };
		};

		struct FVertexHandlerAttributeDescription
		{
			u64 size{ 0ull };
			EVertexAttributeType type{ EVertexAttributeType::eIndex };
		};

		struct FVertexHandlerCreateInfo
		{
			utl::vector<FVertexHandlerAttributeDescription> descriptions{};
			u64 indexSize{ 1ull };
			EVertexHandlerFlags flags{};
			std::string name{};
		};


		struct FVertexDeclarationBindingDescription
		{
			u32 binding{ 0u };
			u32 stride{ 0u };
		};

		struct FVertexDeclarationAttributeDescription
		{
			EVertexAttributeType type{ EVertexAttributeType::eIndex };
			u32 location{ 0u };
			u32 binding{ 0u };;
			EApiDataFormat format{ EApiDataFormat::eUnknown };
			u32 offset{ 0u };
		};

		struct FVertexDeclarationInfo
		{
			utl::vector<FVertexDeclarationBindingDescription> bindingDescriptions{};
			utl::vector<FVertexDeclarationAttributeDescription> attributeDescriptions{};
		};

		struct FVertexRange
		{
			u64 firstLod, lodCount;
		};
	}
}


namespace std
{
	template<>
	struct hash<engine::render::FVertexDeclarationBindingDescription>
	{
		u64 operator()(const engine::render::FVertexDeclarationBindingDescription& desc) const
		{
			return hash<u32>()(desc.binding) ^ (hash<u32>()(desc.stride) << 1);
		}
	};

	template<>
	struct hash<engine::render::FVertexDeclarationAttributeDescription> {
		u64 operator()(const engine::render::FVertexDeclarationAttributeDescription& attr) const
		{
			return hash<u32>()(attr.location) ^
				(hash<u32>()(attr.binding) << 1) ^
				(hash<u32>()(utl::cast<u32>(attr.format)) << 2) ^
				(hash<u32>()(attr.offset) << 3);
		}
	};

	template<>
	struct hash<engine::render::FVertexDeclarationInfo>
	{
		u64 operator()(const engine::render::FVertexDeclarationInfo& info) const
		{
			u64 h = 0ull;
			for (const auto& desc : info.bindingDescriptions)
				h ^= hash<engine::render::FVertexDeclarationBindingDescription>()(desc) + 0x9e3779b9 + (h << 6) + (h >> 2);
			for (const auto& attr : info.attributeDescriptions)
				h ^= hash<engine::render::FVertexDeclarationAttributeDescription>()(attr) + 0x9e3779b9 + (h << 6) + (h >> 2);
			return h;
		}
	};
}