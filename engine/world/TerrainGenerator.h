#pragma once

#include <cstdint>
#include <engine/interface/render/VertexStructures.h>

namespace engine
{
	namespace world
	{
		struct FTerrainVertexAttribute
		{
			render::EVertexAttributeType type{ render::EVertexAttributeType::eIndex };
			std::vector<uint8_t> data{};
			size_t stride{};
			size_t count{};
		};

		class CTerrainGenerator
		{
		public:
			CTerrainGenerator() = default;
			CTerrainGenerator(uint32_t vertices_per_edge, uint32_t width, float uv_scale);
			bool generate(std::vector<FTerrainVertexAttribute>& out_attributes, FTerrainVertexAttribute& out_indices);
		private:
			uint32_t m_verticesPerEdge{};
			uint32_t m_quadsPerSide{};
			size_t m_quadCount{};
			float m_vertexOffset{};
			float m_uvScale{};
		};
	}
}