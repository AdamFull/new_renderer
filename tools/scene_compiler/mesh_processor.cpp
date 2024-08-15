#include "mesh_processor.h"

#include <easygltf/easygltf_loader.h>
#include <easygltf/easygltf_helpers.h>

#include <meshoptimizer.h>

#include <hash.hpp>

using namespace mesh_compiler;

namespace data_type
{
	constexpr const u32 vertex_position = utl::fnv1a_32_hash_cstr(easygltf::base::enumerators::vertex_attribute_type::position);
	constexpr const u32 vertex_normal = utl::fnv1a_32_hash_cstr(easygltf::base::enumerators::vertex_attribute_type::normal);
	constexpr const u32 vertex_texcoord_0 = utl::fnv1a_32_hash_cstr(easygltf::base::enumerators::vertex_attribute_type::texcoord_0);
	constexpr const u32 vertex_texcoord_1 = utl::fnv1a_32_hash_cstr(easygltf::base::enumerators::vertex_attribute_type::texcoord_1);
	constexpr const u32 vertex_color_0 = utl::fnv1a_32_hash_cstr(easygltf::base::enumerators::vertex_attribute_type::color_0);
	constexpr const u32 vertex_color_1 = utl::fnv1a_32_hash_cstr(easygltf::base::enumerators::vertex_attribute_type::color_1);
	constexpr const u32 vertex_tangent = utl::fnv1a_32_hash_cstr(easygltf::base::enumerators::vertex_attribute_type::tangent);
	constexpr const u32 vertex_joints_0 = utl::fnv1a_32_hash_cstr(easygltf::base::enumerators::vertex_attribute_type::joints_0);
	constexpr const u32 vertex_weights_0 = utl::fnv1a_32_hash_cstr(easygltf::base::enumerators::vertex_attribute_type::weights_0);
	constexpr const u32 vertex_custom = utl::fnv1a_32_hash_cstr(easygltf::base::enumerators::vertex_attribute_type::custom);
	constexpr const u32 indices = utl::fnv1a_32_hash_cstr("INDICES");
	constexpr const u32 skin = utl::fnv1a_32_hash_cstr("SKIN");
	constexpr const u32 animation_input = utl::fnv1a_32_hash_cstr("ANIMATION_INPUT");
	constexpr const u32 animation_output = utl::fnv1a_32_hash_cstr("ANIMATION_OUTPUT");
}

u32 get_data_type(const char* name)
{
	return utl::fnv1a_32_hash_cstr(name);
}

const char* get_data_name(u32 type)
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
	case data_type::skin: return "SKIN"; break;
	}

	return "";
}

size_t get_index_stream_optimal_size(size_t size)
{
	if (size < std::numeric_limits<uint8_t>::max())
		return 1ull;
	else if (size < std::numeric_limits<uint16_t>::max())
		return 2ull;
	else if (size < std::numeric_limits<u32>::max())
		return 4ull;

	return 0ull;
}

int32_t get_component_type_by_data_type(u32 type, size_t size)
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
	case data_type::skin: return easygltf::base::enumerators::component_type::float_type; break;
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
	case 4: { 
		switch (component_type)
		{
		case data_type::skin: return easygltf::base::enumerators::accessor_type::mat2_type; break;
		default: return easygltf::base::enumerators::accessor_type::vec4_type; break;
		}
		 
	} break;
	case 9: return easygltf::base::enumerators::accessor_type::mat3_type; break;
	case 16: return easygltf::base::enumerators::accessor_type::mat4_type; break;
	}

	return nullptr;
}

int32_t get_view_target_by_data_type(u32 type)
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

void calculate_accessor_min_and_max(easygltf::Accessor& accessor, const FStream& stream)
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
			case easygltf::base::enumerators::component_type::uint_type: copy_element<u32>(buffer_ptr, value); break;
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

void init_accessor_and_copy_data(easygltf::Accessor& accessor, const FStream& buffer, u32 type, utl::vector<uint8_t>& out)
{
	accessor.byteOffset = out.size();
	accessor.count = buffer.count;
	accessor.componentType = get_component_type_by_data_type(type, buffer.stride);
	accessor.type = get_accessor_type_by_attribute_type(accessor.componentType, buffer.stride);
	calculate_accessor_min_and_max(accessor, buffer);

	out.insert(out.end(), buffer.data.begin(), buffer.data.end());
}

void read_vertex_stream(const std::string& name, const easygltf::Stream& stream, utl::vector<FVertexStream>& vstreams)
{
	auto& vertex_stream = vstreams.emplace_back();
	vertex_stream.type = get_data_type(name.c_str());

	auto& buffer = vertex_stream.buffer;
	buffer.count = stream.count;
	buffer.stride = stream.stride;
	buffer.data.insert(buffer.data.end(), stream.data, stream.data + (stream.stride * stream.count));
}

void read_index_stream(const easygltf::Stream& stream, utl::vector<u32>& indices)
{
	if (stream.stride == 4ull)
	{
		// 32bit index
		utl::vector<u32> temp(stream.count);
		std::memcpy(temp.data(), stream.data, stream.stride * stream.count);
		indices.insert(indices.end(), temp.begin(), temp.end());
	}
	else if (stream.stride == 2ull)
	{
		// 16bit index
		utl::vector<uint16_t> temp(stream.count);
		std::memcpy(temp.data(), stream.data, stream.stride * stream.count);
		indices.insert(indices.end(), temp.begin(), temp.end());
	}
	else if (stream.stride == 1ull)
	{
		// 8bit index
		utl::vector<uint8_t> temp(stream.count);
		std::memcpy(temp.data(), stream.data, stream.stride * stream.count);
		indices.insert(indices.end(), temp.begin(), temp.end());
	}
}

// filterStreams
// mergeMeshes 
// mergeMeshInstances

CGLTFEditor::CGLTFEditor()
{
	meshopt_encodeVertexVersion(0);
	meshopt_encodeIndexVersion(1);
}

bool CGLTFEditor::load(const std::string& path)
{
	easygltf::CGLTFLoader loader{};

	if (!loader.load(path, m_root))
		return false;

	prepare();

	return true;
}

bool CGLTFEditor::save(const std::string& path)
{
	//apply();
	//
	//if (!m_project.save(path))
	//	return false;
	//
	return true;
}

void CGLTFEditor::process()
{
	for (auto& [mesh_id, meshes] : m_meshes)
	{
		filterEmptyMeshes(mesh_id, meshes);

		// TODO: 
		for (auto& mesh : meshes)
		{
			reindexMesh(mesh);
			filterTriangles(mesh);
			simplifyMesh(mesh, 0.8f, true, false);
			optimizeMesh(mesh);
		}

		filterEmptyMeshes(mesh_id, meshes);
	}

	mergeMaterials();
}

// Load all meshes and load local vertex data to proxy objects
void CGLTFEditor::prepare()
{
	for (const auto& node : m_root.nodes)
	{
		// Load meshes
		if (node.mesh != easygltf::base::constants::not_set)
		{
			const auto& mesh = m_root.meshes.at(node.mesh);
			for (const auto& primitive : mesh.primitives)
			{
				auto& nmesh = m_meshes[node.mesh].emplace_back();

				// Load vertex streams
				for (const auto& [name, accessor_id] : primitive.attributes)
				{
					auto stream = m_root.getStream(accessor_id);
					read_vertex_stream(name, stream, nmesh.streams);
				}

				// Load indices
				if (primitive.indices != easygltf::base::constants::not_set)
				{
					auto index_stream = m_root.getStream(primitive.indices);
					read_index_stream(index_stream, nmesh.indices);
				}

				// Load morph data
				for (const auto& target : primitive.targets)
				{
					auto& morph_streams = nmesh.morphs.emplace_back();
					for (const auto& [name, accessor_id] : primitive.attributes)
					{
						auto stream = m_root.getStream(accessor_id);
						read_vertex_stream(name, stream, morph_streams);
					}
				}
			}
		}

		// Load skins
		if (node.skin != easygltf::base::constants::not_set)
		{
			auto& skin = m_root.skins.at(node.skin);
			if (skin.inverseBindMatrices != easygltf::base::constants::not_set)
			{
				auto stream = m_root.getStream(skin.inverseBindMatrices);

				auto& nskin = m_skins[node.skin];
				auto& buffer = nskin.buffer;
				buffer.count = stream.count;
				buffer.stride = stream.stride;
				buffer.data.insert(buffer.data.end(), stream.data, stream.data + (stream.stride * stream.count));
			}
		}
	}

	for (int32_t animation_id = 0; animation_id < m_root.animations.size(); ++animation_id)
	{
		auto& animation = m_root.animations.at(animation_id);
		// Load samplers
		for (const auto& sampler : animation.samplers)
		{
			auto& nsampler = m_samplers[animation_id].emplace_back();

			{
				auto stream = m_root.getStream(sampler.input);
				nsampler.input.count;
				nsampler.input.stride;
				nsampler.input.data.insert(nsampler.input.data.end(), stream.data, stream.data + (stream.stride * stream.count));
			}

			{
				auto stream = m_root.getStream(sampler.output);
				nsampler.output.count;
				nsampler.output.stride;
				nsampler.output.data.insert(nsampler.output.data.end(), stream.data, stream.data + (stream.stride * stream.count));
			}
		}
	}
}

void CGLTFEditor::apply()
{
	std::unordered_map<u32, utl::vector<uint8_t>> buffer_map{};
	std::unordered_map<u32, int32_t> buffer_view_map{};

	// Helper functions
	auto add_buffer_view = [this, &buffer_view_map](u32 type)
		{
			if (!buffer_view_map.count(type))
			{
				buffer_view_map[type] = static_cast<int32_t>(m_root.bufferViews.size());
				m_root.bufferViews.emplace_back();
			}
		};

	auto apply_accessor_and_data = [this, &buffer_map, &buffer_view_map](const FStream& buffer, u32 type) -> int32_t
		{
			int32_t last_accessor = static_cast<int32_t>(m_root.accessors.size());

			auto& temp_buffer = buffer_map[type];
			auto& accessor = m_root.accessors.emplace_back();
			accessor.bufferView = buffer_view_map[type];
			init_accessor_and_copy_data(accessor, buffer, type, temp_buffer);

			return last_accessor;
		};

	for (const auto& [mesh_idx, primitives] : m_meshes)
	{
		auto& mesh = m_root.meshes.at(mesh_idx);
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
				add_buffer_view(stream.type);
				auto attribute_name = get_data_name(stream.type);
				mesh_primitive.attributes[attribute_name] = apply_accessor_and_data(stream.buffer, stream.type);
			}

			// Create morph accessors
			for (const auto& morph_streams : primitive.morphs)
			{
				auto& mesh_morph = mesh_primitive.targets.emplace_back();
				for (const auto& stream : morph_streams)
				{
					add_buffer_view(stream.type);
					auto attribute_name = get_data_name(stream.type);
					mesh_morph[attribute_name] = apply_accessor_and_data(stream.buffer, stream.type);
				}
			}

			if (!primitive.indices.empty())
			{
				FStream buffer{};
				buffer.stride = get_index_stream_optimal_size(primitive.indices.size());
				buffer.count = primitive.indices.size();

				add_buffer_view(data_type::indices);

				switch (buffer.stride)
				{
				case 1ull: {
					buffer.data.insert(buffer.data.end(), primitive.indices.begin(), primitive.indices.end());
				} break;
				case 2ull: {
					utl::vector<uint16_t> temp{};
					temp.insert(temp.end(), primitive.indices.begin(), primitive.indices.end());

					buffer.data.resize(buffer.count * buffer.stride);
					std::memcpy(&buffer.data[0], &temp[0], buffer.data.size());
				} break;
				case 4ull: {
					buffer.data.resize(buffer.count * buffer.stride);
					std::memcpy(&buffer.data[0], &primitive.indices[0], buffer.data.size());
				} break;
				}

				mesh_primitive.indices = apply_accessor_and_data(buffer, data_type::indices);
			}
		}
	}

	// Process all skins
	for (auto& [skin_idx, skin] : m_skins)
	{
		auto& mesh_skin = m_root.skins[skin_idx];
		add_buffer_view(data_type::skin);
		mesh_skin.inverseBindMatrices = apply_accessor_and_data(skin.buffer, data_type::skin);
	}

	// Process all anims
	for (auto& [anim_idx, samplers] : m_samplers)
	{
		auto& animation = m_root.animations[anim_idx];
		assert(samplers.size() == animation.samplers.size());

		for (size_t idx = 0ull; idx < animation.samplers.size(); ++idx)
		{
			auto& anim_sampler = animation.samplers[idx];
			auto& sampler = samplers[idx];

			add_buffer_view(data_type::animation_input);
			anim_sampler.input = apply_accessor_and_data(sampler.input, data_type::animation_input);

			add_buffer_view(data_type::animation_output);
			anim_sampler.input = apply_accessor_and_data(sampler.output, data_type::animation_output);
		}
	}

	// TODO: idk about this, but in future i should create buffers in not this way
	auto& buffer = m_root.buffers.emplace_back();

	// Fill buffer and create buffer views
	for (const auto& [type, temp_buffer] : buffer_map)
	{
		auto buffer_view_idx = buffer_view_map[type];
		auto& buffer_view = m_root.bufferViews.at(buffer_view_idx);

		buffer_view.buffer = 0;
		buffer_view.byteOffset = buffer.data.size();
		buffer_view.byteLength = temp_buffer.size();
		buffer_view.target = get_view_target_by_data_type(type);

		buffer.data.insert(buffer.data.end(), temp_buffer.begin(), temp_buffer.end());
	}

	buffer.byteLength = buffer.data.size();
}

void CGLTFEditor::mergeMaterials()
{
	utl::vector<int32_t> remapped(m_root.materials.size());
	for (size_t idx = 0ull; idx < m_root.materials.size(); ++idx)
	{
		remapped[idx] = idx;

		if (!m_root.materials[idx].name.empty())
			continue;

		for (size_t jdx = 0ull; jdx < idx; ++jdx)
		{
			if (!m_root.materials[jdx].name.empty())
				continue;

			if (m_root.materials[idx] == m_root.materials[jdx])
			{
				remapped[idx] = jdx;
				break;
			}
		}
	}

	for (size_t idx = 0ull; idx < m_root.meshes.size(); ++idx)
	{
		auto& mesh = m_root.meshes[idx];

		for (auto& primitive : mesh.primitives)
		{
			if (primitive.material != easygltf::base::constants::not_set)
				primitive.material = remapped[primitive.material];

			if (primitive.hasExtension<easygltf::extensions::KHR_materials_variants>())
			{
				auto& ext = primitive.khrMaterialVariants;
				for (auto& mapping : ext.mappings)
				{
					if (mapping.material != easygltf::base::constants::not_set)
						mapping.material = remapped[mapping.material];
				}
			}
		}
	}
}

void CGLTFEditor::filterEmptyMeshes(int32_t mesh_id, utl::vector<FMesh>& meshes)
{
	auto& nmesh = m_root.meshes[mesh_id];

	size_t writen{ 0ull };
	for (size_t idx = 0ull; idx < meshes.size(); ++idx)
	{
		auto& mesh = meshes[idx];

		if (mesh.streams.empty())
			continue;

		if (mesh.streams[0].buffer.data.empty())
			continue;

		if (mesh.indices.empty())
			continue;

		utl::vector<FVertexStream> streams{};
		streams.swap(mesh.streams);

		utl::vector<u32> indices{};
		indices.swap(mesh.indices);

		meshes[writen] = mesh;
		meshes[writen].streams.swap(streams);
		meshes[writen].indices.swap(indices);

		// Same thing for mesh primitive
		auto& primitive = nmesh.primitives[idx];

		std::unordered_map<std::string, int32_t> attributes;
		attributes.swap(primitive.attributes);

		int32_t index{ primitive.indices };

		nmesh.primitives[writen] = primitive;
		nmesh.primitives[writen].attributes.swap(attributes);
		nmesh.primitives[writen].indices = index;

		writen++;
	}

	meshes.resize(writen);
}

void CGLTFEditor::reindexMesh(FMesh& mesh)
{
	size_t total_vertices{ 0ull };
	utl::vector<meshopt_Stream> streams{};

	for (auto& stream : mesh.streams)
	{
		auto& buffer = stream.buffer;
		streams.emplace_back(meshopt_Stream{ buffer.get(), buffer.stride, buffer.stride });
		total_vertices = buffer.count;
	}

	if (streams.empty())
		return;

	utl::vector<u32> remapped(total_vertices);
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

void CGLTFEditor::filterTriangles(FMesh& mesh)
{
	size_t writen{ 0ull };
	for (size_t idx = 0ull; idx < mesh.indices.size(); idx += 3ull)
	{
		u32 a = mesh.indices[idx];
		u32 b = mesh.indices[idx + 1ull];
		u32 c = mesh.indices[idx + 2ull];

		if (a != b && a != c && b != c)
		{
			mesh.indices[writen] = a;
			mesh.indices[writen + 1ull] = b;
			mesh.indices[writen + 2ull] = c;
			writen += 3ull;
		}
	}

	mesh.indices.resize(writen);
}

void CGLTFEditor::simplifyMesh(FMesh& mesh, float threshold, bool agressive, bool lock_borders)
{
	if (mesh.indices.empty())
		return;

	auto positions = std::find_if(mesh.streams.begin(), mesh.streams.end(), [](const FVertexStream& stream) { return stream.type == data_type::vertex_position; });
	if (positions == mesh.streams.end())
		return;

	size_t vertex_count = positions->buffer.count;

	size_t target_index_count = static_cast<size_t>(static_cast<float>(mesh.indices.size() / 3ull) * threshold) * 3ull;
	float target_error{ 1e-2f };
	float target_error_aggressive{ 1e-1f };
	u32 options = lock_borders ? meshopt_SimplifyLockBorder : 0u;

	if (target_index_count < 1ull)
		return;

	float* vertices = reinterpret_cast<float*>(positions->buffer.get());

	utl::vector<u32> new_indices(mesh.indices.size());
	new_indices.resize(meshopt_simplify(&new_indices[0], &mesh.indices[0], mesh.indices.size(), vertices, positions->buffer.count, positions->buffer.stride, target_index_count, target_error, options));
	mesh.indices.swap(new_indices);

	if (agressive && mesh.indices.size() > target_index_count)
	{
		new_indices.resize(meshopt_simplify(&new_indices[0], &mesh.indices[0], mesh.indices.size(), vertices, positions->buffer.count, positions->buffer.stride, target_index_count, target_error_aggressive, options));
		mesh.indices.swap(new_indices);
	}
}

void CGLTFEditor::optimizeMesh(FMesh& mesh, bool compressmore)
{
	if (mesh.indices.empty())
		return;

	size_t vertex_count = mesh.streams.front().buffer.count;

	if (compressmore)
		meshopt_optimizeVertexCacheStrip(&mesh.indices[0], &mesh.indices[0], mesh.indices.size(), vertex_count);
	else
		meshopt_optimizeVertexCache(&mesh.indices[0], &mesh.indices[0], mesh.indices.size(), vertex_count);

	utl::vector<u32> remapped(vertex_count);
	size_t unique_vertices = meshopt_optimizeVertexFetchRemap(&remapped[0], &mesh.indices[0], mesh.indices.size(), vertex_count);
	assert(unique_vertices <= vertex_count);

	meshopt_remapIndexBuffer(&mesh.indices[0], &mesh.indices[0], mesh.indices.size(), &remapped[0]);

	for (auto& stream : mesh.streams)
	{
		auto& buffer = stream.buffer;
		assert(buffer.count == vertex_count);

		meshopt_remapVertexBuffer(&buffer.data[0], &buffer.data[0], buffer.count, buffer.stride, &remapped[0]);
		buffer.data.resize(unique_vertices * buffer.stride);
		buffer.count = unique_vertices;
	}
}

bool CGLTFEditor::checkMergeAbility(int32_t mesh_id, FMesh& lhs, FMesh& rhs)
{

	return true;
}

void CGLTFEditor::mergeMeshes(FMesh& target, const FMesh& mesh)
{
	assert(target.streams.size() == mesh.streams.size());

	size_t vertex_offset = target.streams[0].buffer.count;
	size_t index_offset = target.indices.size();

	for (size_t idx = 0; idx < target.streams.size(); ++idx)
	{
		auto& tstream = target.streams[idx];
		auto& mstream = mesh.streams[idx];
		tstream.buffer.data.insert(tstream.buffer.data.end(), mstream.buffer.data.begin(), mstream.buffer.data.end());
		tstream.buffer.count += mstream.buffer.count;
	}

	target.indices.resize(target.indices.size() + mesh.indices.size());

	size_t index_count = mesh.indices.size();

	for (size_t idx = 0ull; idx < index_count; ++idx)
		target.indices[index_offset + idx] = static_cast<u32>(vertex_offset + mesh.indices[idx]);
}

void CGLTFEditor::mergeMeshes(int32_t mesh_id, utl::vector<FMesh>& meshes)
{
	for (size_t idx = 0ull; idx < meshes.size(); ++idx)
	{
		FMesh& target = meshes[idx];

		if (target.streams.empty())
			continue;

		size_t target_vertices = target.streams[0].buffer.count;
		size_t target_indices = target.indices.size();

		size_t last_merged = idx;

		for (size_t jdx = idx + 1ull; jdx < meshes.size(); ++jdx)
		{
			FMesh& mesh = meshes[jdx];

			if (!mesh.streams.empty() && checkMergeAbility(mesh_id, target, mesh))
			{
				target_vertices += mesh.streams[0].buffer.count;
				target_indices += mesh.indices.size();
				last_merged = jdx;
			}
		}

		for (size_t jdx = 0ull; jdx < target.streams.size(); ++jdx)
		{
			auto& stream = target.streams[jdx];
			auto& buffer = stream.buffer;
			buffer.data.reserve(target_vertices * buffer.stride);
			buffer.count = target_vertices;
		}

		target.indices.reserve(target_indices);

		for (size_t jdx = idx + 1ull; jdx <= last_merged; ++jdx)
		{
			FMesh& mesh = meshes[jdx];

			if (!mesh.streams.empty() && checkMergeAbility(mesh_id, target, mesh))
			{
				mergeMeshes(target, mesh);

				mesh.streams.clear();
				mesh.indices.clear();
				//mesh.nodes.clear();
				//mesh.instances.clear();
			}
		}

		assert(target.streams[0].buffer.count == target_vertices);
		assert(target.indices.size() == target_indices);
	}
}