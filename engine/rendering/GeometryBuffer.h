#pragma once

#include <vector>
#include <gfx/GFXStructures.h>

namespace engine
{
	namespace gfx
	{
		class Context;
	}

	namespace render
	{
		enum class VertexAttributeType : u32
		{
			ePosition,
			eColor,
			eTexCoord,
			eNormal,
			eTangent,
			eJoint,
			eWeight
		};

		struct LodRange
		{
			uint32_t lod_start{ 0u };
			uint32_t lod_count{ 0u };
		};

		class GeometryBuffer final
		{
		public:
			GeometryBuffer(gfx::Context& context);
			~GeometryBuffer();

			void AddVertexStream(VertexAttributeType type, const uint8_t* data, uint32_t stride, uint32_t count);
		private:
			gfx::Context& context;

			std::vector<gfx::Buffer*> vertex_streams{};
			gfx::VertexLayout vertex_layout{};
		};
	}
}