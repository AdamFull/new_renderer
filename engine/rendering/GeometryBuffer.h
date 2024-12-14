#pragma once

#include <vector>

#include <edge/gfx/gfx_structures.h>
#include "GeometryBufferInterface.h"

namespace engine
{
	namespace render
	{
		class InterleavedGeometryBuffer final : public GeometryBuffer
		{
		public:
			InterleavedGeometryBuffer(edge::gfx::ResourceLoader& loader, const GeometryBufferCreateInfo& create_info);
			~InterleavedGeometryBuffer();

			void Flush() override;

			void BindVertexBuffers(edge::gfx::CommandList& command_list) override;

			uint64_t GetBufferAddress(VertexAttributeFlag flag = {}) override;

			VertexRange AddMesh(const std::array<AttributeStream, k_max_attribure_description>& attributes, const IndexStream& indices, shlib::Mesh& mesh) override;

			void AddVertexStreamData(const void* data, uint32_t count, uint32_t stride, VertexAttributeFlag flag = {}) override;
		private:
			VertexData vertex_data{};
		};

		class DeinterleavedGeometryBuffer final : public GeometryBuffer
		{
		public:
			DeinterleavedGeometryBuffer(edge::gfx::ResourceLoader& loader, const GeometryBufferCreateInfo& create_info);
			~DeinterleavedGeometryBuffer();

			void Flush() override;

			void BindVertexBuffers(edge::gfx::CommandList& command_list) override;

			virtual uint64_t GetBufferAddress(VertexAttributeFlag flag = {}) override;

			VertexRange AddMesh(const std::array<AttributeStream, k_max_attribure_description>& attributes, const IndexStream& indices, shlib::Mesh& mesh) override;

			void AddVertexStreamData(const void* data, uint32_t count, uint32_t stride, VertexAttributeFlag flags = {}) override;
		private:
			std::array<VertexData, k_max_attribure_description> vertex_datas{};
		};
	}
}