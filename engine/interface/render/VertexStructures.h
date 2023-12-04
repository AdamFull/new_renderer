#pragma once

#include <vector>

namespace engine
{
	namespace render
	{
		enum class EVertexAttributeType : uint32_t
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

		struct FVertexAttributePointer
		{
			EVertexAttributeType type{ EVertexAttributeType::eIndex };
			const uint8_t* data{ nullptr };
			size_t stride{ 0ull };
			size_t count{ 0ull };
		};

		struct FVertexBufferBindingInfo
		{
			uint32_t firstBinding{ 0u };
			std::vector<size_t> buffers{};
			std::vector<size_t> offsets{};
		};

		struct FIndexBufferBindingInfo
		{
			size_t bufferId{ ~0ull };
			size_t offset{ 0ull };
			size_t indexSize{ 0ull };
		};

		struct FVertexHandlerAttributeDescription
		{
			size_t size{ 0ull };
			EVertexAttributeType type{ EVertexAttributeType::eIndex };
		};

		struct FVertexHandlerCreateInfo
		{
			std::vector<FVertexHandlerAttributeDescription> descriptions{};
			size_t indexSize{ 1ull };
			std::string name{};
		};


		struct FVertexDeclarationBindingDescription
		{
			uint32_t binding{ 0u };
			uint32_t stride{ 0u };
		};

		struct FVertexDeclarationAttributeDescription
		{
			EVertexAttributeType type{ EVertexAttributeType::eIndex };
			uint32_t location{ 0u };
			uint32_t binding{ 0u };;
			uint32_t format{ 0u };
			uint32_t offset{ 0u };
		};

		struct FVertexDeclarationInfo
		{
			std::vector<FVertexDeclarationBindingDescription> bindingDescriptions{};
			std::vector<FVertexDeclarationAttributeDescription> attributeDescriptions{};
		};

		class IVertexHandler
		{
		public:
			virtual ~IVertexHandler() = default;

			virtual void create(const FVertexHandlerCreateInfo& create_info) = 0;
			virtual void allocate() = 0;

			virtual void bind(uint32_t queue) = 0;

			virtual size_t addVertices(const std::vector<FVertexAttributePointer>& attributes_data) = 0;
			virtual size_t addIndices(const FVertexAttributePointer& indices, size_t vertex_offset) = 0;

			virtual size_t getLastVertex() = 0;
			virtual size_t getLastIndex() = 0;

			virtual const FVertexDeclarationInfo& getVertexDeclaration() const = 0;
			virtual size_t getHash() const = 0;
			virtual const std::string& getDefinitions() const = 0;

			virtual size_t getVertexHandle() const = 0;
			virtual size_t getIndexHandle() const = 0;
		};
	}
}


namespace std
{
	template<>
	struct hash<engine::render::FVertexDeclarationBindingDescription>
	{
		size_t operator()(const engine::render::FVertexDeclarationBindingDescription& desc) const
		{
			return hash<uint32_t>()(desc.binding) ^ (hash<uint32_t>()(desc.stride) << 1);
		}
	};

	template<>
	struct hash<engine::render::FVertexDeclarationAttributeDescription> {
		size_t operator()(const engine::render::FVertexDeclarationAttributeDescription& attr) const
		{
			return hash<uint32_t>()(attr.location) ^
				(hash<uint32_t>()(attr.binding) << 1) ^
				(hash<uint32_t>()(attr.format) << 2) ^
				(hash<uint32_t>()(attr.offset) << 3);
		}
	};

	template<>
	struct hash<engine::render::FVertexDeclarationInfo>
	{
		size_t operator()(const engine::render::FVertexDeclarationInfo& info) const
		{
			size_t h = 0;
			for (const auto& desc : info.bindingDescriptions)
				h ^= hash<engine::render::FVertexDeclarationBindingDescription>()(desc) + 0x9e3779b9 + (h << 6) + (h >> 2);
			for (const auto& attr : info.attributeDescriptions)
				h ^= hash<engine::render::FVertexDeclarationAttributeDescription>()(attr) + 0x9e3779b9 + (h << 6) + (h >> 2);
			return h;
		}
	};
}