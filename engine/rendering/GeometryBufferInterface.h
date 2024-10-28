#pragma once

#include <unordered_map>
#include <enum.hpp>
#include <engine/rendering/gfx/Structures.h>

namespace engine
{
	namespace gfx
	{
		class Context;
	}

	namespace render
	{
		static constexpr uint32_t k_max_attribure_description{ 7u };

		enum class VertexAttributeFlag : uint32_t
		{
			ePosition = 1 << 0,
			eColor = 1 << 1,
			eTexCoord = 1 << 2,
			eNormal = 1 << 3,
			eTangent = 1 << 4,
			eJoint = 1 << 5,
			eWeight = 1 << 6
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(VertexAttributeFlags, VertexAttributeFlag);

		struct AttributeDescription
		{
			VertexAttributeFlag attribute_flag{};
			gfx::Format format{};
		};

		struct IndexDescription
		{
			gfx::IndexType type{ gfx::IndexType::eUint16 };
		};

		struct GeometryBufferCreateInfo
		{
			std::vector<AttributeDescription> attribute_descriptions{};
			IndexDescription index_description{};
		};

		struct VertexData
		{
			uint32_t count{ 0u };
			gfx::Buffer* buffer{ nullptr };
			std::vector<uint8_t> data{};
		};

		struct IndexData
		{
			uint32_t count{ 0u };
			gfx::Buffer* buffer{ nullptr };
			gfx::IndexType type{};
			std::vector<uint8_t> data{};
		};

		inline uint32_t GetVertexFormatSizeBytes(gfx::Format format)
		{
			switch (format)
			{
			case gfx::Format::eR8G8B8A8Unorm: return 4u; break;
			case gfx::Format::eR32G32Sfloat:
			case gfx::Format::eR16G16B16A16Unorm: return 8u; break;
			case gfx::Format::eR32G32B32Sfloat: return 12u; break;
			case gfx::Format::eR32G32B32A32Sfloat: return 16u; break;
			default:
				break;
			}

			return 1u;
		}

		inline uint32_t GetVertexAttributeLocation(VertexAttributeFlag flag)
		{
			switch (flag)
			{
			case engine::render::VertexAttributeFlag::ePosition: return 0u; break;
			case engine::render::VertexAttributeFlag::eColor: return 1u; break;
			case engine::render::VertexAttributeFlag::eTexCoord: return 2u; break;
			case engine::render::VertexAttributeFlag::eNormal: return 3u; break;
			case engine::render::VertexAttributeFlag::eTangent: return 4u; break;
			case engine::render::VertexAttributeFlag::eJoint: return 5u; break;
			case engine::render::VertexAttributeFlag::eWeight: return 6u; break;
			default:
				break;
			}

			return ~0u;
		}

		class IGeometryBuffer
		{
		public:
			virtual ~IGeometryBuffer() = default;

			virtual void Flush() = 0;

			virtual void Bind(gfx::CommandList& command_list) = 0;

			virtual uint64_t GetBufferAddress(VertexAttributeFlag flag = {}) = 0;

			uint64_t GetIndexBufferAddress()
			{
				return index_data.buffer->GetAddress();
			}

			virtual void AddVertexStreamData(const void* data, uint32_t count, uint32_t stride, VertexAttributeFlag flag = {}) = 0;

			void AddIndexStreamData(const void* data, uint32_t count, uint32_t stride)
			{
				index_data.data.insert(index_data.data.end(), static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + count * stride);
				index_data.count += count;
			}

			gfx::VertexLayout const& GetVertexLayout() const
			{
				return vertex_layout;
			}
		protected:
			gfx::VertexLayout vertex_layout{};
			IndexData index_data{};
		};
	}
}