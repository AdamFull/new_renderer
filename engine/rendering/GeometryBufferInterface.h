#pragma once

#include <engine/rendering/gfx/ResourceLoader.h>

#include <unordered_map>
#include <enum.hpp>

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

		struct AttributeStream
		{
			VertexAttributeFlag attribute_flag{};
			std::span<const uint8_t> byte_range{};
			uint32_t count{ 0u };
		};

		struct IndexStream
		{
			std::span<const uint8_t> byte_range{};
			uint32_t count{ 0u };
		};

		struct VertexRange
		{
			uint32_t first_index{};
			uint32_t index_count{};
			uint32_t first_vertex{};
			uint32_t vertex_count{};
		};

		uint32_t Hash(const AttributeDescription& desc, uint32_t hash = 0u);
		uint32_t Hash(const IndexDescription& desc, uint32_t hash = 0u);
		uint32_t Hash(const GeometryBufferCreateInfo& create_info, uint32_t hash = 0u);

		class GeometryBuffer
		{
		public:
			GeometryBuffer(gfx::ResourceContext& resource_context);
			virtual ~GeometryBuffer();

			virtual void Flush();

			void BindIndexBuffer(gfx::CommandList& command_list);

			virtual void BindVertexBuffers(gfx::CommandList& command_list) = 0;

			void Bind(gfx::CommandList& command_list);

			virtual uint64_t GetBufferAddress(VertexAttributeFlag flag = {}) = 0;

			uint64_t GetIndexBufferAddress();

			virtual VertexRange AddMesh(const std::array<AttributeStream, k_max_attribure_description>& attributes, const IndexStream& indices) = 0;

			virtual void AddVertexStreamData(const void* data, uint32_t count, uint32_t stride, VertexAttributeFlag flag = {}) = 0;

			void AddIndexStreamData(const void* data, uint32_t count, uint32_t stride);

			gfx::VertexLayout const& GetVertexLayout() const;
		protected:
			void TryFlushBuffer(gfx::Buffer** buffer_ptr, const std::vector<uint8_t>& data, gfx::BufferUsageFlags usage);
		protected:
			gfx::ResourceContext& resource_context;
			gfx::VertexLayout vertex_layout{};
			IndexData index_data{};
		};
	}
}