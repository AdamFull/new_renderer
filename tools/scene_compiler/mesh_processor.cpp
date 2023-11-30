#include "mesh_processor.h"

#include <easygltf/easygltf.h>
#include <meshoptimizer.h>

#include <udetail.hpp>

using namespace mesh_compiler;

enum EVertexAttributeType : uint32_t
{
	VERTEX_ATTRIBUTE_POSITION =		utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::position),
	VERTEX_ATTRIBUTE_NORMAL =		utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::normal),
	VERTEX_ATTRIBUTE_TEXCOORD_0 =	utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::texcoord_0),
	VERTEX_ATTRIBUTE_TEXCOORD_1 =	utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::texcoord_1),
	VERTEX_ATTRIBUTE_COLOR_0 =		utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::color_0),
	VERTEX_ATTRIBUTE_COLOR_1 =		utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::color_1),
	VERTEX_ATTRIBUTE_TANGENT =		utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::tangent),
	VERTEX_ATTRIBUTE_JOINTS_0 =		utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::joints_0),
	VERTEX_ATTRIBUTE_WEIGHTS_0 =	utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::weights_0),
	VERTEX_ATTRIBUTE_CUSTOM =		utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::custom)
};

uint32_t get_vertex_attribute_type(const char* name)
{
	return utl::const_hash(name);
}

const char* get_vertex_attribute_name(uint32_t type)
{
	switch (type)
	{
	case VERTEX_ATTRIBUTE_POSITION: return easygltf::base::enumerators::vertex_attribute_type::position; break;
	case VERTEX_ATTRIBUTE_NORMAL: return easygltf::base::enumerators::vertex_attribute_type::normal; break;
	case VERTEX_ATTRIBUTE_TEXCOORD_0: return easygltf::base::enumerators::vertex_attribute_type::texcoord_0; break;
	case VERTEX_ATTRIBUTE_TEXCOORD_1: return easygltf::base::enumerators::vertex_attribute_type::texcoord_1; break;
	case VERTEX_ATTRIBUTE_COLOR_0: return easygltf::base::enumerators::vertex_attribute_type::color_0; break;
	case VERTEX_ATTRIBUTE_COLOR_1: return easygltf::base::enumerators::vertex_attribute_type::color_1; break;
	case VERTEX_ATTRIBUTE_TANGENT: return easygltf::base::enumerators::vertex_attribute_type::tangent; break;
	case VERTEX_ATTRIBUTE_JOINTS_0: return easygltf::base::enumerators::vertex_attribute_type::joints_0; break;
	case VERTEX_ATTRIBUTE_WEIGHTS_0: return easygltf::base::enumerators::vertex_attribute_type::weights_0; break;
	case VERTEX_ATTRIBUTE_CUSTOM: return easygltf::base::enumerators::vertex_attribute_type::custom; break;
	}
}

size_t get_index_stream_optimal_size(size_t size)
{
	if (size < std::numeric_limits<uint8_t>::max())
		return 1ull;
	else if (size < std::numeric_limits<uint16_t>::max())
		return 2ull;
	else if (size < std::numeric_limits<uint32_t>::max())
		return 4ull;

	return 0ull;
}

void reindex_mesh(FMesh& mesh)
{
	auto total_vertices = mesh.streams.front().count;

	std::vector<meshopt_Stream> streams{};
	for (auto& stream : mesh.streams)
		streams.emplace_back(meshopt_Stream{ stream.data.data(), stream.stride, stream.stride });

	if (streams.empty())
		return;

	std::vector<uint32_t> remapped(total_vertices);
	size_t unique_vertices = meshopt_generateVertexRemapMulti(remapped.data(), mesh.indices.data(), mesh.indices.size(), total_vertices, streams.data(), streams.size());
	assert(unique_vertices <= total_vertices);

	meshopt_remapIndexBuffer(mesh.indices.data(), mesh.indices.data(), mesh.indices.size(), remapped.data());

	for (auto& stream : mesh.streams)
	{
		assert(stream.count == total_vertices);
		meshopt_remapVertexBuffer(stream.data.data(), stream.data.data(), total_vertices, stream.stride, remapped.data());
		stream.data.resize(unique_vertices);
	}
}

CMeshPorcessor::CMeshPorcessor()
{
	meshopt_encodeVertexVersion(0);
	meshopt_encodeIndexVersion(1);
}

// Load all meshes and load local vertex data to proxy objects
void CMeshPorcessor::prepare(easygltf::CGLTFProject* project)
{
	auto& root = project->getGltf();

	for (const auto& node : root.nodes)
	{
		if (node.mesh != easygltf::base::constants::not_set)
		{
			const auto& mesh = root.meshes.at(node.mesh);
			for (const auto& primitive : mesh.primitives)
			{
				auto& nmesh = m_meshes.emplace_back();
				nmesh.meshIndex = node.mesh;

				// Load vertex streams
				for (const auto& [name, accessor_id] : primitive.attributes)
				{
					auto stream = project->getStream(accessor_id);

					auto& vertex_stream = nmesh.streams.emplace_back();
					vertex_stream.count = stream.count;
					vertex_stream.stride = stream.stride;
					vertex_stream.type = get_vertex_attribute_type(name.c_str());
					vertex_stream.data = std::vector<uint8_t>(stream.data, stream.data + (stream.stride * stream.count));
				}

				// Load indices
				if (primitive.indices != easygltf::base::constants::not_set)
				{
					auto index_stream = project->getStream(primitive.indices);

					if (index_stream.stride == 4ull)
					{
						// 32bit index
						std::vector<uint32_t> temp(index_stream.count);
						std::memcpy(temp.data(), index_stream.data, index_stream.stride * index_stream.count);
						nmesh.indices = temp;
					}
					else if (index_stream.stride == 2ull)
					{
						// 16bit index
						std::vector<uint16_t> temp(index_stream.count);
						std::memcpy(temp.data(), index_stream.data, index_stream.stride * index_stream.count);
						nmesh.indices = std::vector<uint32_t>(temp.begin(), temp.end());
					}
					else if(index_stream.stride == 1ull)
					{
						// 8bit index
						std::vector<uint8_t> temp(index_stream.count);
						std::memcpy(temp.data(), index_stream.data, index_stream.stride * index_stream.count);
						nmesh.indices = std::vector<uint32_t>(temp.begin(), temp.end());
					}
				}
			}
		}
	}
}

void CMeshPorcessor::process()
{
	for (auto& mesh : m_meshes)
	{
		reindex_mesh(mesh);
	}
}

void CMeshPorcessor::apply(easygltf::CGLTFProject* project)
{

}