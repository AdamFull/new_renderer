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
			stl::vector<u64> buffers{};
			stl::vector<u64> offsets{};
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
			stl::vector<FVertexHandlerAttributeDescription> descriptions{};
			u64 indexSize{ 1ull };
			stl::string name{};
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
			u32 format{ 0u };
			u32 offset{ 0u };
		};

		struct FVertexDeclarationInfo
		{
			stl::vector<FVertexDeclarationBindingDescription> bindingDescriptions{};
			stl::vector<FVertexDeclarationAttributeDescription> attributeDescriptions{};
		};

		struct FVertexRange
		{
			u64 firstIndex, indexCount;
			u64 firstVertex, vertexCount;
			u64 firstLod, lodCount;
		};

		class IVertexHandler
		{
		public:
			virtual ~IVertexHandler() = default;

			virtual void create(const FVertexHandlerCreateInfo& create_info) = 0;
			virtual void allocate() = 0;

			virtual void bind(EQueueTypeFlagBits queue) = 0;

			virtual const FVertexRange& addMeshData(const stl::vector<FVertexAttributePointer>& attributes_data, const FVertexAttributePointer& indices) = 0;

			virtual u64 addVertices(const stl::vector<FVertexAttributePointer>& attributes_data) = 0;
			virtual u64 addIndices(const FVertexAttributePointer& indices) = 0;

			virtual u64 getLastVertex() = 0;
			virtual u64 getLastIndex() = 0;

			virtual const FVertexDeclarationInfo& getVertexDeclaration() const = 0;
			virtual u64 getHash() const = 0;
			virtual const stl::string& getDefinitions() const = 0;

			virtual const std::unique_ptr<IGPUVector>& getGeometryBuffer(EGeometryBufferIndices buffer_index) = 0;

			virtual u64 getVertexHandle() const = 0;
			virtual u64 getIndexHandle() const = 0;
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
				(hash<u32>()(attr.format) << 2) ^
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