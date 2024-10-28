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
		class InterleavedGeometryBuffer final : public IGeometryBuffer
		{
		public:
			InterleavedGeometryBuffer(gfx::Context& context, const GeometryBufferCreateInfo& create_info);
			~InterleavedGeometryBuffer();

			void Flush() override;

			void Bind(gfx::CommandList& command_list) override;

			uint64_t GetBufferAddress(VertexAttributeFlag flag = {}) override;

			void AddVertexStreamData(const void* data, uint32_t count, uint32_t stride, VertexAttributeFlag flag = {}) override;
		private:
			gfx::Context& context;

			VertexData vertex_data{};
		};
	}
}