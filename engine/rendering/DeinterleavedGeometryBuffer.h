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
		class DeinterleavedGeometryBuffer final : public IGeometryBuffer
		{
		public:
			DeinterleavedGeometryBuffer(gfx::Context& context, const GeometryBufferCreateInfo& create_info);
			~DeinterleavedGeometryBuffer();

			void Bind(gfx::CommandList& command_list) override;

			virtual uint64_t GetBufferAddress(VertexAttributeFlag flag = {}) override;

			void AddVertexStreamData(const void* data, uint32_t count, uint32_t stride, VertexAttributeFlag flags = {}) override;
		private:
			gfx::Context& context;

			std::array<VertexData, k_max_attribure_description> vertex_datas{};
		};
	}
}