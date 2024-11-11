#include "TerrainGenerator.h"

/*
using namespace engine::world;

struct FTerrainVertex
{
	glm::vec3 pos{};
	glm::vec2 uv{};
	glm::vec3 normal{};
};

CTerrainGenerator::CTerrainGenerator(uint32_t vertices_per_edge, uint32_t width, float uv_scale)
{
	m_verticesPerEdge = vertices_per_edge;
	m_quadsPerSide = m_verticesPerEdge - 1u;
	m_quadCount = m_quadsPerSide * m_quadsPerSide;
	m_vertexOffset = static_cast<float>(width) / static_cast<float>(m_quadsPerSide);
	m_uvScale = uv_scale;
}

bool CTerrainGenerator::generate(utl::vector<FTerrainVertexAttribute>& out_attributes, FTerrainVertexAttribute& out_indices)
{
	const uint32_t triangle_count = m_quadCount * 2u;

	const float vert_per_edge = static_cast<float>(m_verticesPerEdge);

	utl::vector<FTerrainVertex> vertices{};
	for (uint32_t x = 0u; x < m_verticesPerEdge; ++x)
	{
		for (uint32_t z = 0u; z < m_verticesPerEdge; ++z)
		{
			FTerrainVertex vertex{};
			vertex.pos.x = x * m_vertexOffset + m_vertexOffset / 2.0f - vert_per_edge * m_vertexOffset / 2.0f;
			vertex.pos.y = 0;
			vertex.pos.z = z * m_vertexOffset + m_vertexOffset / 2.0f - vert_per_edge * m_vertexOffset / 2.0f;
			vertex.uv = glm::vec2(static_cast<float>(x) / vert_per_edge, static_cast<float>(z) / vert_per_edge) * -m_uvScale;
			vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f);
			vertices.emplace_back(vertex);
		}
	}

	utl::vector<uint32_t> indices(m_quadCount * 6u);
	for (uint32_t x = 0ull; x < m_quadsPerSide; ++x)
	{
		for (uint32_t y = 0ull; y < m_quadsPerSide; ++y)
		{
			uint32_t index = (x + y * m_quadsPerSide) * 6u;
			indices[index] = (x + y * m_verticesPerEdge);					// bottom left
			indices[index + 1ull] = indices[index] + m_verticesPerEdge;		// bottom right
			indices[index + 2ull] = indices[index + 1ull] + 1u;				// top right
			indices[index + 3ull] = indices[index] + 1u;					// top left
			indices[index + 4ull] = (x + y * m_verticesPerEdge);			// bottom left
			indices[index + 5ull] = indices[index + 1ull] + 1u;				// top right
		}
	}

	{
		auto& positions = out_attributes.emplace_back();
		positions.type = render::EVertexAttributeType::ePosition;
		positions.stride = sizeof(FTerrainVertex::pos);
		positions.count = vertices.size();
		positions.data.resize(positions.stride * positions.count);
	}

	{
		auto& uvs = out_attributes.emplace_back();
		uvs.type = render::EVertexAttributeType::eTexCoord;
		uvs.stride = sizeof(FTerrainVertex::uv);
		uvs.count = vertices.size();
		uvs.data.resize(uvs.stride * uvs.count);
	}

	{
		auto& norm = out_attributes.emplace_back();
		norm.type = render::EVertexAttributeType::eNormal;
		norm.stride = sizeof(FTerrainVertex::normal);
		norm.count = vertices.size();
		norm.data.resize(norm.stride * norm.count);
	}

	auto& positions = out_attributes[0];
	auto& uvs = out_attributes[1];
	auto& normals = out_attributes[2];

	// Copying vertex data
	for (size_t idx = 0ull; idx < vertices.size(); ++idx)
	{
		const auto& vertex = vertices[idx];
		util::Copy(&positions.data[idx * positions.stride], &vertex.pos, positions.stride);
		util::Copy(&uvs.data[idx * uvs.stride], &vertex.uv, uvs.stride);
		util::Copy(&normals.data[idx * normals.stride], &vertex.normal, normals.stride);
	}

	// Copying index data
	out_indices.count = indices.size();

	if (positions.count > std::numeric_limits<uint16_t>::max())
	{
		out_indices.stride = sizeof(uint32_t);
		out_indices.data.resize(out_indices.count * out_indices.stride);
		util::Copy(&out_indices.data[0], &indices[0], out_indices.data.size());
	}
	else
	{
		out_indices.stride = sizeof(uint16_t);

		utl::vector<uint16_t> temp{};
		temp.insert(temp.end(), indices.begin(), indices.end());
		out_indices.data.resize(out_indices.count * out_indices.stride);
		util::Copy(&out_indices.data[0], &temp[0], out_indices.data.size());
	}

	return true;
}
*/