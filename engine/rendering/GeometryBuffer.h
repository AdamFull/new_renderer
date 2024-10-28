#pragma once

#include <vector>
#include <gfx/Structures.h>
#include <engine/rendering/GeometryBufferInterface.h>

namespace engine
{
	namespace gfx
	{
		class Context;
	}

	namespace render
	{
		class InterleavedGeometryBuffer final : public GeometryBuffer
		{
		public:
			InterleavedGeometryBuffer(gfx::ResourceContext& context, const GeometryBufferCreateInfo& create_info);
			~InterleavedGeometryBuffer();

			void Flush() override;

			void Bind(gfx::CommandList& command_list) override;

			uint64_t GetBufferAddress(VertexAttributeFlag flag = {}) override;

			VertexRange AddMesh(const std::array<AttributeStream, k_max_attribure_description>& attributes, const IndexStream& indices) override;

			void AddVertexStreamData(const void* data, uint32_t count, uint32_t stride, VertexAttributeFlag flag = {}) override;
		private:
			VertexData vertex_data{};
		};

		class DeinterleavedGeometryBuffer final : public GeometryBuffer
		{
		public:
			DeinterleavedGeometryBuffer(gfx::ResourceContext& context, const GeometryBufferCreateInfo& create_info);
			~DeinterleavedGeometryBuffer();

			void Flush() override;

			void Bind(gfx::CommandList& command_list) override;

			virtual uint64_t GetBufferAddress(VertexAttributeFlag flag = {}) override;

			VertexRange AddMesh(const std::array<AttributeStream, k_max_attribure_description>& attributes, const IndexStream& indices) override;

			void AddVertexStreamData(const void* data, uint32_t count, uint32_t stride, VertexAttributeFlag flags = {}) override;
		private:
			std::array<VertexData, k_max_attribure_description> vertex_datas{};
		};
	}
}