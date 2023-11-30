#include "mesh_processor.h"

#include <easygltf/easygltf.h>
#include <easygltf/easygltf_helpers.h>

#include <meshoptimizer.h>

#include <udetail.hpp>

using namespace mesh_compiler;

namespace data_type
{
	constexpr const uint32_t vertex_position = utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::position);
	constexpr const uint32_t vertex_normal = utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::normal);
	constexpr const uint32_t vertex_texcoord_0 = utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::texcoord_0);
	constexpr const uint32_t vertex_texcoord_1 = utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::texcoord_1);
	constexpr const uint32_t vertex_color_0 = utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::color_0);
	constexpr const uint32_t vertex_color_1 = utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::color_1);
	constexpr const uint32_t vertex_tangent = utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::tangent);
	constexpr const uint32_t vertex_joints_0 = utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::joints_0);
	constexpr const uint32_t vertex_weights_0 = utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::weights_0);
	constexpr const uint32_t vertex_custom = utl::const_hash(easygltf::base::enumerators::vertex_attribute_type::custom);
	constexpr const uint32_t indices = utl::const_hash("INDICES");
}

uint32_t get_data_type(const char* name)
{
	return utl::const_hash(name);
}

const char* get_data_name(uint32_t type)
{
	switch (type)
	{
	case data_type::vertex_position: return easygltf::base::enumerators::vertex_attribute_type::position; break;
	case data_type::vertex_normal: return easygltf::base::enumerators::vertex_attribute_type::normal; break;
	case data_type::vertex_texcoord_0: return easygltf::base::enumerators::vertex_attribute_type::texcoord_0; break;
	case data_type::vertex_texcoord_1: return easygltf::base::enumerators::vertex_attribute_type::texcoord_1; break;
	case data_type::vertex_color_0: return easygltf::base::enumerators::vertex_attribute_type::color_0; break;
	case data_type::vertex_color_1: return easygltf::base::enumerators::vertex_attribute_type::color_1; break;
	case data_type::vertex_tangent: return easygltf::base::enumerators::vertex_attribute_type::tangent; break;
	case data_type::vertex_joints_0: return easygltf::base::enumerators::vertex_attribute_type::joints_0; break;
	case data_type::vertex_weights_0: return easygltf::base::enumerators::vertex_attribute_type::weights_0; break;
	case data_type::vertex_custom: return easygltf::base::enumerators::vertex_attribute_type::custom; break;
	case data_type::indices: return "INDICES"; break;
	}

	return "";
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

int32_t get_component_type_by_data_type(uint32_t type, size_t size)
{
	switch (type)
	{
	case data_type::vertex_position:
	case data_type::vertex_normal:
	case data_type::vertex_texcoord_0:
	case data_type::vertex_texcoord_1:
	case data_type::vertex_color_0:
	case data_type::vertex_color_1:
	case data_type::vertex_tangent:
	case data_type::vertex_weights_0:
	case data_type::vertex_custom: return easygltf::base::enumerators::component_type::float_type; break;
	case data_type::vertex_joints_0: return easygltf::base::enumerators::component_type::ushort_type; break;
	case data_type::indices: {
		switch (size)
		{
		case 1ull: return easygltf::base::enumerators::component_type::ubyte_type; break;
		case 2ull: return easygltf::base::enumerators::component_type::ushort_type; break;
		case 4ull: return easygltf::base::enumerators::component_type::uint_type; break;
		}
	} break;
	}
}

const char* get_accessor_type_by_attribute_type(int32_t component_type, size_t stride)
{
	auto component_size = easygltf::get_size_by_component_type(component_type);
	auto element_count = stride / component_size;

	switch (element_count)
	{
	case 1: return easygltf::base::enumerators::accessor_type::scalar_type; break;
	case 2: return easygltf::base::enumerators::accessor_type::vec2_type; break;
	case 3: return easygltf::base::enumerators::accessor_type::vec3_type; break;
	case 4: return easygltf::base::enumerators::accessor_type::vec4_type; break;
	}

	return nullptr;
}

int32_t get_view_target_by_data_type(uint32_t type)
{
	switch (type)
	{
	case data_type::vertex_position:
	case data_type::vertex_normal:
	case data_type::vertex_texcoord_0:
	case data_type::vertex_texcoord_1:
	case data_type::vertex_color_0:
	case data_type::vertex_color_1:
	case data_type::vertex_tangent:
	case data_type::vertex_weights_0:
	case data_type::vertex_joints_0:
	case data_type::vertex_custom: return easygltf::base::enumerators::buffer_view_type::array_buffer; break;
	case data_type::indices: return easygltf::base::enumerators::buffer_view_type::element_array_buffer; break;
	}

	return -1;
}

template<class _Ty>
void copy_element(const uint8_t* src, double& dst)
{
	if constexpr(std::is_same_v<_Ty, double>)
		std::memcpy(&dst, src, sizeof(_Ty));
	else
	{
		_Ty temp{};
		std::memcpy(&temp, src, sizeof(_Ty));
		dst = static_cast<double>(temp);
	}
}

void calculate_accessor_min_and_max(easygltf::Accessor& accessor, const FBuffer& stream)
{
	auto component_size = easygltf::get_size_by_component_type(accessor.componentType);
	auto element_count = easygltf::get_element_count_by_accessor_type(accessor.type.c_str());

	accessor.min.assign(element_count, std::numeric_limits<double>::max());
	accessor.max.assign(element_count, std::numeric_limits<double>::lowest());

	for (size_t idx = 0ull; idx < stream.count; ++idx)
	{
		size_t offset = idx * stream.stride;
		for (size_t element = 0ull; element < element_count; ++element)
		{
			size_t element_offset = offset + element * component_size;
			double value{ 0.0 };

			auto* buffer_ptr = &stream.data[element_offset];
			switch (accessor.componentType)
			{
			case easygltf::base::enumerators::component_type::byte_type: copy_element<int8_t>(buffer_ptr, value); break;
			case easygltf::base::enumerators::component_type::ubyte_type: copy_element<uint8_t>(buffer_ptr, value); break;
			case easygltf::base::enumerators::component_type::short_type: copy_element<int16_t>(buffer_ptr, value); break;
			case easygltf::base::enumerators::component_type::ushort_type: copy_element<uint16_t>(buffer_ptr, value); break;
			case easygltf::base::enumerators::component_type::int_type: copy_element<int32_t>(buffer_ptr, value); break;
			case easygltf::base::enumerators::component_type::uint_type: copy_element<uint32_t>(buffer_ptr, value); break;
			case easygltf::base::enumerators::component_type::float_type: copy_element<float>(buffer_ptr, value); break;
			case easygltf::base::enumerators::component_type::double_type: copy_element<double>(buffer_ptr, value); break;
			}

			auto& min_value = accessor.min[element];
			auto& max_value = accessor.max[element];
			
			min_value = std::min(min_value, value);
			max_value = std::max(max_value, value);
		}
	}
}

void init_accessor_and_copy_data(easygltf::Accessor& accessor, const FBuffer& buffer, uint32_t type, std::vector<uint8_t>& out)
{
	accessor.byteOffset = out.size();
	accessor.count = buffer.count;
	accessor.componentType = get_component_type_by_data_type(type, buffer.stride);
	accessor.type = get_accessor_type_by_attribute_type(accessor.componentType, buffer.stride);
	calculate_accessor_min_and_max(accessor, buffer);

	out.insert(out.end(), buffer.data.begin(), buffer.data.end());
}

void read_vertex_stream(const std::string& name, const easygltf::Stream& stream, std::vector<FVertexStream>& vstreams)
{
	auto& vertex_stream = vstreams.emplace_back();
	vertex_stream.type = get_data_type(name.c_str());

	auto& buffer = vertex_stream.buffer;
	buffer.count = stream.count;
	buffer.stride = stream.stride;
	buffer.data.insert(buffer.data.end(), stream.data, stream.data + (stream.stride * stream.count));
}

void read_index_stream(const easygltf::Stream& stream, std::vector<uint32_t>& indices)
{
	if (stream.stride == 4ull)
	{
		// 32bit index
		std::vector<uint32_t> temp(stream.count);
		std::memcpy(temp.data(), stream.data, stream.stride * stream.count);
		indices.insert(indices.end(), temp.begin(), temp.end());
	}
	else if (stream.stride == 2ull)
	{
		// 16bit index
		std::vector<uint16_t> temp(stream.count);
		std::memcpy(temp.data(), stream.data, stream.stride * stream.count);
		indices.insert(indices.end(), temp.begin(), temp.end());
	}
	else if (stream.stride == 1ull)
	{
		// 8bit index
		std::vector<uint8_t> temp(stream.count);
		std::memcpy(temp.data(), stream.data, stream.stride * stream.count);
		indices.insert(indices.end(), temp.begin(), temp.end());
	}
}

void reindex_mesh(FMesh& mesh)
{
	size_t total_vertices{ 0ull };
	std::vector<meshopt_Stream> streams{};

	for (auto& stream : mesh.streams)
	{
		auto& buffer = stream.buffer;
		streams.emplace_back(meshopt_Stream{ buffer.get(), buffer.stride, buffer.stride });
		total_vertices = buffer.count;
	}

	if (streams.empty())
		return;

	std::vector<uint32_t> remapped(total_vertices);
	size_t unique_vertices = meshopt_generateVertexRemapMulti(&remapped[0], &mesh.indices[0], mesh.indices.size(), total_vertices, &streams[0], streams.size());
	assert(unique_vertices <= total_vertices);

	meshopt_remapIndexBuffer(&mesh.indices[0], &mesh.indices[0], mesh.indices.size(), &remapped[0]);

	for (auto& stream : mesh.streams)
	{
		auto& buffer = stream.buffer;
		assert(buffer.count == total_vertices);
		meshopt_remapVertexBuffer(buffer.get(), buffer.get(), total_vertices, buffer.stride, &remapped[0]);
		buffer.data.resize(unique_vertices * buffer.stride);
		buffer.count = unique_vertices;
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
		// Load meshes
		if (node.mesh != easygltf::base::constants::not_set)
		{
			const auto& mesh = root.meshes.at(node.mesh);
			for (const auto& primitive : mesh.primitives)
			{
				auto& nmesh = m_meshes[node.mesh].emplace_back();

				// Load vertex streams
				for (const auto& [name, accessor_id] : primitive.attributes)
				{
					auto stream = project->getStream(accessor_id);
					read_vertex_stream(name, stream, nmesh.streams);
				}

				// Load indices
				if (primitive.indices != easygltf::base::constants::not_set)
				{
					auto index_stream = project->getStream(primitive.indices);
					read_index_stream(index_stream, nmesh.indices);
				}

				// Load morph data
				for (const auto& target : primitive.targets)
				{
					auto& morph_streams = nmesh.morphs.emplace_back();
					for (const auto& [name, accessor_id] : primitive.attributes)
					{
						auto stream = project->getStream(accessor_id);
						read_vertex_stream(name, stream, morph_streams);
					}
				}
			}
		}

		// Load skins
		if (node.skin != easygltf::base::constants::not_set)
		{
			auto& skin = root.skins.at(node.skin);
			if (skin.inverseBindMatrices != easygltf::base::constants::not_set)
			{
				auto stream = project->getStream(skin.inverseBindMatrices);

				auto& nskin = m_skins[node.skin];
				auto& buffer = nskin.buffer;
				buffer.count = stream.count;
				buffer.stride = stream.stride;
				buffer.data.insert(buffer.data.end(), stream.data, stream.data + (stream.stride * stream.count));
			}
		}
	}

	for (int32_t animation_id = 0; animation_id < root.animations.size(); ++animation_id)
	{
		auto& animation = root.animations.at(animation_id);
		// Load samplers
		for (const auto& sampler : animation.samplers)
		{
			auto& nsampler = m_samplers[animation_id].emplace_back();

			{
				auto stream = project->getStream(sampler.input);
				nsampler.input.count;
				nsampler.input.stride;
				nsampler.input.data.insert(nsampler.input.data.end(), stream.data, stream.data + (stream.stride * stream.count));
			}

			{
				auto stream = project->getStream(sampler.output);
				nsampler.output.count;
				nsampler.output.stride;
				nsampler.output.data.insert(nsampler.output.data.end(), stream.data, stream.data + (stream.stride * stream.count));
			}
		}
	}
}

void CMeshPorcessor::process()
{
	for (auto& [mesh_id, meshes] : m_meshes)
	{
		for (auto& mesh : meshes)
		{
			reindex_mesh(mesh);
		}
	}
}

void CMeshPorcessor::apply(easygltf::CGLTFProject* project)
{
	std::unordered_map<uint32_t, std::vector<uint8_t>> buffer_map{};
	std::unordered_map<uint32_t, int32_t> buffer_view_map{};

	// TODO: think about how to create buffer views
	auto& root = project->getGltf();

	for (const auto& [mesh_idx, primitives] : m_meshes)
	{
		auto& mesh = root.meshes.at(mesh_idx);
		assert(mesh.primitives.size() == primitives.size());

		for (size_t idx = 0ull; idx < primitives.size(); ++idx)
		{
			auto& mesh_primitive = mesh.primitives.at(idx);
			auto& primitive = primitives.at(idx);

			mesh_primitive.attributes.clear();
			mesh_primitive.targets.clear();

			// Create attribute accessors
			for (const auto& stream : primitive.streams)
			{
				if (!buffer_view_map.count(stream.type))
				{
					buffer_view_map[stream.type] = static_cast<int32_t>(root.bufferViews.size());
					root.bufferViews.emplace_back();
				}

				auto attribute_name = get_data_name(stream.type);
				mesh_primitive.attributes[attribute_name] = static_cast<int32_t>(root.accessors.size());

				auto& temp_buffer = buffer_map[stream.type];

				auto& accessor = root.accessors.emplace_back();
				accessor.bufferView = buffer_view_map[stream.type];
				init_accessor_and_copy_data(accessor, stream.buffer, stream.type, temp_buffer);
			}

			// Create morph accessors
			for (const auto& morph_streams : primitive.morphs)
			{
				auto& mesh_morph = mesh_primitive.targets.emplace_back();
				for (const auto& stream : morph_streams)
				{
					if (!buffer_view_map.count(stream.type))
					{
						buffer_view_map[stream.type] = static_cast<int32_t>(root.bufferViews.size());
						root.bufferViews.emplace_back();
					}

					auto attribute_name = get_data_name(stream.type);
					mesh_morph[attribute_name] = static_cast<int32_t>(root.accessors.size());

					auto& temp_buffer = buffer_map[stream.type];

					auto& accessor = root.accessors.emplace_back();
					accessor.bufferView = buffer_view_map[stream.type];
					init_accessor_and_copy_data(accessor, stream.buffer, stream.type, temp_buffer);
					init_accessor_and_copy_data(root.accessors.emplace_back(), stream.buffer, stream.type, temp_buffer);
				}
			}

			if (!primitive.indices.empty())
			{
				mesh_primitive.indices = static_cast<int32_t>(root.accessors.size());

				FBuffer buffer{};
				buffer.stride = get_index_stream_optimal_size(primitive.indices.size());
				buffer.count = primitive.indices.size();

				if (!buffer_view_map.count(data_type::indices))
				{
					buffer_view_map[data_type::indices] = static_cast<int32_t>(root.bufferViews.size());
					root.bufferViews.emplace_back();
				}

				switch (buffer.stride)
				{
				case 1ull: {
					buffer.data.insert(buffer.data.end(), primitive.indices.begin(), primitive.indices.end());
				} break;
				case 2ull: {
					std::vector<uint16_t> temp{};
					temp.insert(temp.end(), primitive.indices.begin(), primitive.indices.end());

					buffer.data.resize(buffer.count * buffer.stride);
					std::memcpy(&buffer.data[0], &temp[0], buffer.data.size());
				} break;
				case 4ull: {
					buffer.data.resize(buffer.count * buffer.stride);
					std::memcpy(&buffer.data[0], &primitive.indices[0], buffer.data.size());
				} break;
				}

				auto& temp_buffer = buffer_map[data_type::indices];

				auto& accessor = root.accessors.emplace_back();
				accessor.bufferView = buffer_view_map[data_type::indices];
				init_accessor_and_copy_data(accessor, buffer, data_type::indices, temp_buffer);
			}
		}
	}

	// TODO: idk about this, but in future i should create buffers in not this way
	auto& buffer = root.buffers.emplace_back();

	// Fill buffer and create buffer views
	for (const auto& [type, temp_buffer] : buffer_map)
	{
		auto buffer_view_idx = buffer_view_map[type];
		auto& buffer_view = root.bufferViews.at(buffer_view_idx);

		buffer_view.buffer = 0;
		buffer_view.byteOffset = buffer.data.size();
		buffer_view.byteLength = temp_buffer.size();
		buffer_view.target = get_view_target_by_data_type(type);

		buffer.data.insert(buffer.data.end(), temp_buffer.begin(), temp_buffer.end());
	}

	buffer.byteLength = buffer.data.size();
}