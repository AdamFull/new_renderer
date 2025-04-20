#pragma once

#include <vector>

#include <edge/gfx/gfx_structures.h>
#include "GeometryBufferInterface.h"

namespace engine::render {
	class InterleavedGeometryBuffer final : public GeometryBuffer {
	public:
		InterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info);
		~InterleavedGeometryBuffer();

		void flush() override;

		void bind_vertex_buffers(edge::gfx::ICommandList& command_list) override;

		auto get_buffer_address(VertexAttributeFlag flag = {}) -> uint64_t override;

		auto add_mesh(const std::array<AttributeStream, k_max_attribure_description>& attributes, const IndexStream& indices) -> VertexRange override;

		void add_vertex_stream_data(const void* data, uint32_t count, uint32_t stride, VertexAttributeFlag flag = {}) override;
	private:
		GeometryData vertex_data{};
	};

	class DeinterleavedGeometryBuffer final : public GeometryBuffer {
	public:
		DeinterleavedGeometryBuffer(const GeometryBufferCreateInfo& create_info);
		~DeinterleavedGeometryBuffer();

		void flush() override;

		void bind_vertex_buffers(edge::gfx::ICommandList& command_list) override;

		virtual auto get_buffer_address(VertexAttributeFlag flag = {}) -> uint64_t override;

		auto add_mesh(const std::array<AttributeStream, k_max_attribure_description>& attributes, const IndexStream& indices) -> VertexRange override;

		void add_vertex_stream_data(const void* data, uint32_t count, uint32_t stride, VertexAttributeFlag flags = {}) override;
	private:
		std::array<GeometryData, k_max_attribure_description> vertex_datas{};
	};
}