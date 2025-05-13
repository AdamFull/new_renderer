#include "Scene.h"

#include "ecs/components/components.h"
#include "ecs/systems/systems.h"

#include "../rendering/Rendering.h"
#include <edge/gfx/gfx_context.h>

#include <edge/util/utl_base.h>
#include <edge/fs/fs_helper.h>
#include <edge/util/Logger.h>
#include <glm/gtc/type_ptr.hpp>

#include <edge/fs/fs_virtual_filesystem.h>
#include <edge/util/utl_profiler.h>

#include "EntityManager.h"

#include "../rendering/Globals.h"

#include "../../assets/shaders/include/interop.h"

#include <cgltf.h>
#include <meshoptimizer.h>
#include <mikktspace.h>

namespace engine::world {
	auto get_address_mode(cgltf_wrap_mode wrap_mode) -> edge::gfx::SamplerAddressMode {
		edge::gfx::SamplerAddressMode addr_mode{ edge::gfx::SamplerAddressMode::eClampToBorder };
		switch (wrap_mode)
		{
		case cgltf_wrap_mode_clamp_to_edge: addr_mode = edge::gfx::SamplerAddressMode::eClampToEdge; break;
		case cgltf_wrap_mode_mirrored_repeat: addr_mode = edge::gfx::SamplerAddressMode::eMirror; break;
		case cgltf_wrap_mode_repeat: addr_mode = edge::gfx::SamplerAddressMode::eRepeat; break;
		default: break;
		}

		return addr_mode;
	}

	edge::gfx::SamplerCreateInfo get_sampler_create_info(const cgltf_sampler& sampler) {
		edge::gfx::SamplerCreateInfo create_info{};
		create_info.max_anisotropy = 16.0f;

		create_info.address_u = get_address_mode(sampler.wrap_s);
		create_info.address_v = get_address_mode(sampler.wrap_t);

		switch (sampler.min_filter)
		{
		case cgltf_filter_type_nearest_mipmap_nearest: {
			create_info.min_filter = edge::gfx::Filter::eNearest;
			create_info.mip_map_mode = edge::gfx::Filter::eNearest;
		} break;
		case cgltf_filter_type_linear_mipmap_nearest: {
			create_info.min_filter = edge::gfx::Filter::eLinear;
			create_info.mip_map_mode = edge::gfx::Filter::eNearest;
		} break;
		case cgltf_filter_type_nearest_mipmap_linear: {
			create_info.min_filter = edge::gfx::Filter::eNearest;
			create_info.mip_map_mode = edge::gfx::Filter::eLinear;
		} break;
		case cgltf_filter_type_linear_mipmap_linear: {
			create_info.min_filter = edge::gfx::Filter::eLinear;
			create_info.mip_map_mode = edge::gfx::Filter::eLinear;
		} break;
		default: break;
		}

		switch (sampler.mag_filter)
		{
		case cgltf_filter_type_nearest: create_info.mag_filter = edge::gfx::Filter::eNearest; break;
		case cgltf_filter_type_linear: create_info.mag_filter = edge::gfx::Filter::eLinear; break;
		default: break;
		}

		return create_info;
	}

	Scene::Scene(render::IRenderer& renderer) :
		renderer{ renderer } {
		EntityManager::register_system<ecs::InputSystem>();
		EntityManager::register_system<ecs::AnimationSystem>();
		EntityManager::register_system<ecs::ScenegraphSystem>(renderer);
		EntityManager::register_system<ecs::SkinningSystem>(renderer);

		EntityManager::register_system<ecs::CameraSystem>(renderer);
		EntityManager::register_system<ecs::MeshSystem>(renderer);
		EntityManager::register_system<ecs::ShadowsSystem>(renderer);
		EntityManager::register_system<ecs::LightingSystem>(renderer);
	}

	Scene::~Scene() {
		if (gltf_data) {
			cgltf_free(gltf_data);
			delete gltf_data;
			gltf_data = nullptr;
		}

		if (gltf_options) {
			delete gltf_options;
			gltf_options = nullptr;
		}
	}

	void Scene::Load(const std::string& path) {

		if (!edge::fs::exists(path)) {
			EDGE_LOGE("[Scene Importer] Scene file is not exists: {}", path);
			return;
		}

		std::string gltf_data_str{};
		if (!edge::fs::read_file(path, gltf_data_str)) {
			EDGE_LOGE("[GLTF Scene Importer] Failed to open scene file: {}", path);
			return;
		}

		gltf_options = new cgltf_options();
		gltf_data = new cgltf_data();

		gltf_options->type = cgltf_file_type_gltf;
		gltf_options->file.user_data = this;
		gltf_options->file.read = [](const cgltf_memory_options* memory_options, const cgltf_file_options* file_options,
			const char* path, cgltf_size* size, void** data) -> cgltf_result {
				if (!edge::fs::exists(path)) {
					return cgltf_result_file_not_found;
				}

				auto* file = edge::fs::open_file(path, edge::fs::FileModeFlag::eRead | edge::fs::FileModeFlag::eBin);
				if (!file) {
					return cgltf_result_file_not_found;
				}

				*size = file->size();
				if (!(*size)) {
					return cgltf_result_data_too_short;
				}

				*data = new uint8_t[*size];
				file->read(*data, *size);

				return cgltf_result_success;
			};
		gltf_options->file.release = [](const cgltf_memory_options* memory_options,
			const cgltf_file_options* file_options, void* data) -> void {
				delete data;
				data = nullptr;
			};

		cgltf_result res = cgltf_parse(gltf_options, gltf_data_str.data(), gltf_data_str.size(), &gltf_data);
		if (res != cgltf_result_success) {
			EDGE_LOGE("[GLTF Scene Importer] Failed to load file: {}", path);
			return;
		}

		res = cgltf_load_buffers(gltf_options, gltf_data, path.data());
		if (res != cgltf_result_success) {
			EDGE_LOGE("[GLTF Scene Importer] Failed to load gltf buffers: {}", path);
			return;
		}

		res = cgltf_validate(gltf_data);
		if (res != cgltf_result_success) {
			EDGE_LOGE("[GLTF Scene Importer] Invalid gltf file data: {}", path);
			return;
		}

		scene_root_directory = edge::fs::parent_path(path);

		// Load images
		for (const auto& image : std::span(gltf_data->images, gltf_data->images_count)) {
			// TODO: push image name
			std::string uri = image.uri;
			uri.resize(cgltf_decode_uri(&uri[0]));
			uri = scene_root_directory + "/" + uri;
			resources.images.push_back(renderer.add_render_resource(uri));
		}

		// Load samplers
		for (const auto& sampler : std::span(gltf_data->samplers, gltf_data->samplers_count)) {
			auto create_info = get_sampler_create_info(sampler);
			resources.samplers.push_back(renderer.add_render_resource(create_info));
		}

		auto get_texture = [this](cgltf_data* gltf, cgltf_texture* gltf_texture, interop::Texture& mat_tex) {
			if (!gltf_texture) {
				return;
			}

			if (gltf_texture->has_basisu) {
				mat_tex.texture = resources.images[(uint16_t)cgltf_image_index(gltf, gltf_texture->basisu_image)];
			}
			else if (gltf_texture->has_webp) {
				mat_tex.texture = resources.images[(uint16_t)cgltf_image_index(gltf, gltf_texture->webp_image)];
			}
			else {
				mat_tex.texture = resources.images[(uint16_t)cgltf_image_index(gltf, gltf_texture->image)];
			}

			mat_tex.sampler = resources.samplers[(uint16_t)cgltf_sampler_index(gltf, gltf_texture->sampler)];
		};

		// Load materials 
		for (const auto& material : std::span(gltf_data->materials, gltf_data->materials_count)) {
			interop::Material new_material{};

			if (material.has_pbr_metallic_roughness) {
				auto& pbr_metallic_roughness = material.pbr_metallic_roughness;

				get_texture(gltf_data, pbr_metallic_roughness.base_color_texture.texture, new_material.base_color);
				new_material.base_color_factor = glm::make_vec4(pbr_metallic_roughness.base_color_factor);

				get_texture(gltf_data, pbr_metallic_roughness.metallic_roughness_texture.texture, new_material.metallic_roughness);
				new_material.metal_rough_factor.x = pbr_metallic_roughness.metallic_factor;
				new_material.metal_rough_factor.y = pbr_metallic_roughness.roughness_factor;
			}

			get_texture(gltf_data, material.normal_texture.texture, new_material.normal);
			get_texture(gltf_data, material.emissive_texture.texture, new_material.emissive);
			get_texture(gltf_data, material.occlusion_texture.texture, new_material.occlusion);

			new_material.emissive_factor = glm::make_vec3(material.emissive_factor);
			if (material.has_emissive_strength) {
				new_material.emissive_factor *= material.emissive_strength.emissive_strength;
			}

			new_material.alpha_mode = static_cast<interop::AlphaMode>(material.alpha_mode);
			new_material.alpha_cutoff = material.alpha_cutoff;

			renderer.add_material(new_material);
		}

		const auto& scene = gltf_data->scene ? *gltf_data->scene : gltf_data->scenes[0];
		auto scene_root = EntityManager::create_root(scene.name);

		for (const auto* node : std::span(scene.nodes, scene.nodes_count)) {
			load_node(node, scene_root);
		}

		render::GeometryBufferCreateInfo geometry_ci{};
		geometry_ci.attribute_descriptions.emplace_back(render::VertexAttributeFlag::ePosition, edge::gfx::Format::eR16G16B16Sint);
		geometry_ci.attribute_descriptions.emplace_back(render::VertexAttributeFlag::eTangent, edge::gfx::Format::eR16Uint);
		geometry_ci.attribute_descriptions.emplace_back(render::VertexAttributeFlag::eNormal, edge::gfx::Format::eR32Uint);
		geometry_ci.attribute_descriptions.emplace_back(render::VertexAttributeFlag::eTexCoord, edge::gfx::Format::eR16G16Uint);
		geometry_ci.index_description.type = edge::gfx::IndexType::eUint16;

		auto geometry_buffer_id = renderer.request_interleaved_geometry_buffer(geometry_ci);
		auto& geometry_buffer = renderer.lookup_geometry_buffer(geometry_buffer_id);
		geometry_buffer.add_vertex_stream_data(resources.vertices.data(), resources.vertices.size(), sizeof(interop::Vertex));
		resources.vertices.clear();

		std::vector<uint16_t> indices{};
		std::transform(resources.indices.begin(), resources.indices.end(), std::back_inserter(indices), 
			[](auto index) {
				return static_cast<uint16_t>(index);
			});

		geometry_buffer.add_index_stream_data(indices.data(), indices.size(), 2);
		resources.indices.clear();

		auto& render_world = renderer.get_render_world();
		render_world.add_submeshes(resources.submeshes);
		resources.submeshes.clear();

		EntityManager::create();
	}

	void Scene::Update(float delta_time) {
		EDGE_PROFILE_ZONE_SCOPED;
		EntityManager::update(delta_time);
	}

	void Scene::load_node(const cgltf_node* node, const entt::entity parent) {
		auto new_node = EntityManager::create_node(node->name);

		auto node_index = cgltf_node_index(gltf_data, node);
		EDGE_LOGI("[Scene] Loadind node: {}:{}.", node->name, node_index);
		node_to_entity[node_index] = new_node;

		auto& transform = EntityManager::get_component<ecs::TransformComponent>(new_node);
		if (node->has_translation) {
			transform.lposition = glm::make_vec3(node->translation);
		}
		if (node->has_rotation) {
			transform.lrotation = glm::make_quat(node->rotation);
		}
		if (node->has_scale) {
			transform.lscale = glm::make_vec3(node->scale);
		}
		if (node->has_matrix) {
			transform.lmodel = glm::make_mat4(node->matrix);
		}

		// Parse node children
		for (const auto* children : std::span(node->children, node->children_count)) {
			load_node(children, new_node);
		}

		if (node->mesh) {
			load_mesh(node->mesh, new_node);
		}

		if (node->camera) {
			load_camera(node->camera, new_node);
		}

		if (node->light) {
			load_light(node->light, new_node);
		}

		EntityManager::attach_child(parent, new_node);
	}

	void optimize_geometry(std::vector<interop::RawVertex>& vertices, std::vector<uint32_t>& indices) {
		std::vector<uint32_t> remap_buffer(vertices.size());
		auto unique_vertices = meshopt_generateVertexRemap(remap_buffer.data(), indices.data(),
			indices.size(), vertices.data(), vertices.size(), sizeof(interop::RawVertex));

		meshopt_remapVertexBuffer(vertices.data(), vertices.data(), vertices.size(),
			sizeof(interop::RawVertex), remap_buffer.data());
		meshopt_remapIndexBuffer(indices.data(), indices.data(), indices.size(),
			remap_buffer.data());

		vertices.resize(unique_vertices);

		meshopt_optimizeVertexCacheFifo(indices.data(), indices.data(), indices.size(), vertices.size(), 16);
		//meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), vertices.size());
		meshopt_optimizeVertexFetch(vertices.data(), indices.data(), indices.size(),
			vertices.data(), vertices.size(), sizeof(interop::RawVertex));
	}

	uint16_t2 quantize_normal(glm::vec3 n) {
		n /= abs(n.x) + abs(n.y) + abs(n.z);
		float2 xy = float2(n.x, n.y);
		xy = n.z >= 0.0f ? xy : interop::oct_wrap(xy);
		xy = xy * 0.5f + 0.5f;
		return uint16_t2(min(uint32_t2(round(xy * float(1 << 16))), uint32_t2((1u << 16) - 1)));
	}

	uint16_t quantize_tangent(const glm::vec4& t, const glm::vec3& n) {
		float3 t1 = glm::normalize(interop::ortho_vec(n));
		float3 t2 = glm::cross(n, t1);
		float2 xy = float2(glm::dot(float3(t), t1), glm::dot(float3(t), t2));
		float x = xy.x / (abs(xy.x) + abs(xy.y));
		x = xy.y >= 0.0f ? x : interop::sq_wrap(x);
		x = x * 0.25f + 0.5f;
		uint16_t tangent_and_sign = std::min(static_cast<uint16_t>(round(x * float(1 << 15))), static_cast<uint16_t>((1u << 15) - 1));
		tangent_and_sign |= (t.w < 0.0f) ? (1 << 15) : 0;
		return tangent_and_sign;
	}

	void quantize_geometry(const interop::Mesh& mesh, const std::vector<interop::RawVertex>& in_vertices,
		std::vector<interop::Vertex>& out_vertices) {
		glm::mat3 position_encode_mat = interop::make_encode_position_matrix(mesh.bb_enc);
		glm::mat3 normal_encode_mat = glm::transpose(glm::inverse(position_encode_mat));

		for (const auto& raw_vertex : in_vertices) {
			auto& new_vertex = out_vertices.emplace_back();
			new_vertex.position = interop::encode_position(raw_vertex.position, mesh.bb_enc);
			new_vertex.normal = quantize_normal(normal_encode_mat * raw_vertex.normal);
			new_vertex.tangent = quantize_tangent(glm::vec4(normal_encode_mat * glm::vec3(raw_vertex.tangent), raw_vertex.tangent.w), raw_vertex.normal);
			new_vertex.uv = interop::encode_texcoord(raw_vertex.uv, mesh.uv_enc);
			new_vertex.color = interop::encode_color(raw_vertex.color);
		}
	}

	template<typename _Ty>
	void get_range_by_accessor(const cgltf_accessor* accessor, const _Ty*& begin, const _Ty*& end) {
		const uint8_t* element = cgltf_buffer_view_data(accessor->buffer_view) + accessor->offset;
		begin = reinterpret_cast<const _Ty*>(element);
		end = begin + accessor->count;
	}

	void load_vertices(const cgltf_primitive& primitive, const glm::vec3& bb_enc, std::vector<interop::RawVertex>& vertices) {
		// TODO: Avoid copying to temp buffer
		if (const cgltf_accessor* accessor = cgltf_find_accessor(&primitive, cgltf_attribute_type_position, 0)) {
			vertices.resize(accessor->count);

			const glm::vec3* begin{}; const glm::vec3* end{};
			get_range_by_accessor(accessor, begin, end);
			for (auto it = begin; it != end; ++it) {
				vertices[std::distance(begin, it)].position = *it;
			}
		}

		if (const cgltf_accessor* accessor = cgltf_find_accessor(&primitive, cgltf_attribute_type_normal, 0)) {
			const glm::vec3* begin{}; const glm::vec3* end{};
			get_range_by_accessor(accessor, begin, end);
			for (auto it = begin; it != end; ++it) {
				vertices[std::distance(begin, it)].normal = *it;
			}
		}

		// TODO: generate tangents with mikktspace
		if (const cgltf_accessor* accessor = cgltf_find_accessor(&primitive, cgltf_attribute_type_tangent, 0)) {
			assert(cgltf_num_components(accessor->type) == 4);
			const glm::vec4* begin{}; const glm::vec4* end{};
			get_range_by_accessor(accessor, begin, end);
			for (auto it = begin; it != end; ++it) {
				vertices[std::distance(begin, it)].tangent = *it;
			}
		}
		else {
			EDGE_LOGW("[GLTF Scene Importer] Tangent data not found, but required.");
		}

		if (const cgltf_accessor* accessor = cgltf_find_accessor(&primitive, cgltf_attribute_type_texcoord, 0)) {
			const glm::vec2* begin{}; const glm::vec2* end{};
			get_range_by_accessor(accessor, begin, end);
			for (auto it = begin; it != end; ++it) {
				vertices[std::distance(begin, it)].uv = *it;
			}
		}

		if (const cgltf_accessor* accessor = cgltf_find_accessor(&primitive, cgltf_attribute_type_color, 0)) {
			if (cgltf_num_components(accessor->type) == 3) {
				const glm::vec3* begin{}; const glm::vec3* end{};
				get_range_by_accessor(accessor, begin, end);
				for (auto it = begin; it != end; ++it) {
					vertices[std::distance(begin, it)].color = glm::vec4(*it, 1.0f);
				}
			}
			else if (cgltf_num_components(accessor->type) == 4) {
				const glm::vec4* begin{}; const glm::vec4* end{};
				get_range_by_accessor(accessor, begin, end);
				for (auto it = begin; it != end; ++it) {
					vertices[std::distance(begin, it)].color = *it;
				}
			}
		}
		else {
			// Fallback. Set white color by default
			for (auto& vertex : vertices) {
				vertex.color = glm::vec4(1.0f);
			}
		}
	}

	// Calculate whole mesh bounds and encoders
	void compute_mesh_bounds(const cgltf_mesh* mesh, interop::Mesh& gpu_mesh) {
		interop::BoundingBox bb{};
		gpu_mesh.bb_enc = glm::vec3(1.0f);

		for (const auto& primitive : std::span(mesh->primitives, mesh->primitives_count)) {
			if (const cgltf_accessor* accessor = cgltf_find_accessor(&primitive, cgltf_attribute_type_position, 0)) {
				const glm::vec3* begin{}; const glm::vec3* end{};
				get_range_by_accessor(accessor, begin, end);

				for (auto it = begin; it != end; ++it) {
					gpu_mesh.bb_enc = glm::max(gpu_mesh.bb_enc, glm::abs(*it));
					bb.min = glm::min(bb.min, *it);
					bb.max = glm::max(bb.max, *it);
				}
			}
			if (const cgltf_accessor* accessor = cgltf_find_accessor(&primitive, cgltf_attribute_type_texcoord, 0)) {
				const glm::vec2* begin{}; const glm::vec2* end{};
				get_range_by_accessor(accessor, begin, end);

				for (auto it = begin; it != end; ++it) {
					gpu_mesh.uv_enc.min = glm::min(gpu_mesh.uv_enc.min, *it);
					gpu_mesh.uv_enc.max = glm::max(gpu_mesh.uv_enc.max, *it);
				}
			}
		}

		gpu_mesh.bb_enc = glm::exp2(glm::ceil(glm::log2(gpu_mesh.bb_enc)));

		// Round off the minimum and the maximum of the bounding square to the next
		// power of 2 if they are not equal to 0
		{
			// Select a relatively big default square size to avoid log2 NaN
			glm::vec2 p = glm::log2(glm::max(glm::max(-gpu_mesh.uv_enc.min, gpu_mesh.uv_enc.max), 1.0f));
			glm::vec2 bs = glm::exp2(glm::ceil(p));
			gpu_mesh.uv_enc.min = glm::mix(glm::vec2(0.0f), -bs, glm::notEqual(gpu_mesh.uv_enc.min, glm::vec2(0.0f)));
			gpu_mesh.uv_enc.max = glm::mix(glm::vec2(0.0f), bs, glm::notEqual(gpu_mesh.uv_enc.max, glm::vec2(0.0f)));
		}

		gpu_mesh.ebb = interop::encode_bounding_box(bb, gpu_mesh.bb_enc);
	}

	void Scene::load_mesh(const cgltf_mesh* mesh, const entt::entity parent) {
		auto& render_world = renderer.get_render_world();
		// TODO: skinning support
		auto [mesh_id, gpu_mesh] = render_world.create_mesh(0);
		compute_mesh_bounds(mesh, gpu_mesh);

		gpu_mesh.submesh_offset = static_cast<uint32_t>(resources.submeshes.size());

		auto& mesh_component = EntityManager::add_component<ecs::MeshComponent>(parent, ecs::MeshComponent{});
		mesh_component.mesh = mesh_id;

		// Create encode matrices for mesh
		glm::mat3 position_encode_mat = interop::make_encode_position_matrix(gpu_mesh.bb_enc);
		glm::mat3 normal_encode_mat = interop::adjugate(position_encode_mat);

		for (const auto& primitive : std::span(mesh->primitives, mesh->primitives_count)) {
			std::vector<interop::RawVertex> vertices{};
			load_vertices(primitive, gpu_mesh.bb_enc, vertices);

			std::vector<uint32_t> indices{};
			if (primitive.indices) {
				indices.resize(primitive.indices->count);
				cgltf_accessor_unpack_indices(primitive.indices, indices.data(), 4, indices.size());
			}

			interop::BoundingBox bb{};
			for (const auto& vertex : vertices) {
				bb.min = glm::min(bb.min, vertex.position);
				bb.max = glm::max(bb.max, vertex.position);
			}

			auto& new_submesh = resources.submeshes.emplace_back();
			new_submesh.first_vertex = static_cast<int32_t>(resources.vertices.size());
			new_submesh.vertex_count = static_cast<uint32_t>(vertices.size());
			new_submesh.material = cgltf_material_index(gltf_data, primitive.material);
			new_submesh.ebb = interop::encode_bounding_box(bb, gpu_mesh.bb_enc);

			// Preprocess new geometry
			optimize_geometry(vertices, indices);

			float lod_scale = meshopt_simplifyScale((const float*)vertices.data(), vertices.size(), sizeof(interop::RawVertex));

			auto* byte_ptr = reinterpret_cast<const uint8_t*>(vertices.data());
			const float* pos_ptr = reinterpret_cast<const float*>(byte_ptr + offsetof(interop::RawVertex, position));
			const float* normal_ptr = reinterpret_cast<const float*>(byte_ptr + offsetof(interop::RawVertex, normal));
			size_t stride = sizeof(interop::RawVertex);

			std::vector<uint32_t> lod_indices = indices;
			float lod_error = 0.f;

			float normal_weights[3] = { 1.f, 1.f, 1.f };
			while (new_submesh.lod_count < constants::k_max_lod_count) {
				auto& new_lod = new_submesh.lods[new_submesh.lod_count++];
				new_lod.first_index = static_cast<uint32_t>(resources.indices.size());
				new_lod.index_count = static_cast<uint32_t>(lod_indices.size());
				new_lod.error = lod_error * lod_scale;

				resources.indices.insert(resources.indices.end(), lod_indices.begin(), lod_indices.end());

				if (new_submesh.lod_count < constants::k_max_lod_count) {
					const float max_error = 1e-1f;
					const uint32_t options = 0;

					size_t next_indices_target = (static_cast<size_t>(static_cast<double>(lod_indices.size()) * 0.65) / 3) * 3;
					float next_error = 0.f;

					size_t next_indices = meshopt_simplifyWithAttributes(lod_indices.data(),
						lod_indices.data(), lod_indices.size(), pos_ptr, vertices.size(), stride,
						normal_ptr, stride, normal_weights, 3, nullptr, next_indices_target,
						max_error, options, &next_error);
					assert(next_indices <= lod_indices.size());

					if (next_indices == lod_indices.size() || next_indices == 0) {
						break;
					}

					if (next_indices >= static_cast<size_t>(static_cast<double>(lod_indices.size()) * 0.95)) {
						break;
					}

					lod_indices.resize(next_indices);
					lod_error = std::max(lod_error, next_error);

					meshopt_optimizeVertexCacheFifo(lod_indices.data(), lod_indices.data(), lod_indices.size(), vertices.size(), 16);
				}
			}

			quantize_geometry(gpu_mesh, vertices, resources.vertices);
		}

		gpu_mesh.submesh_count = static_cast<uint32_t>(resources.submeshes.size()) - gpu_mesh.submesh_offset;
	}

	void Scene::load_camera(const cgltf_camera* camera, const entt::entity parent) {
		ecs::CameraComponent camera_component;
		if (camera->type == cgltf_camera_type_orthographic) {
			camera_component.ctype = ecs::ECameraType::eOrthographic;
			camera_component.xmag = camera->data.orthographic.xmag;
			camera_component.ymag = camera->data.orthographic.ymag;
			camera_component.nearPlane = camera->data.orthographic.znear;
			camera_component.farPlane = camera->data.orthographic.zfar;
		}
		else if (camera->type == cgltf_camera_type_perspective) {
			camera_component.ctype = ecs::ECameraType::ePerspective;
			camera_component.aspectRatio = camera->data.perspective.aspect_ratio;
			camera_component.fieldOfView = static_cast<float>(glm::degrees(2.0f * glm::atan(glm::tan(camera->data.perspective.yfov * 0.5) * camera->data.perspective.aspect_ratio)));
			camera_component.nearPlane = camera->data.perspective.znear;
			camera_component.farPlane = camera->data.perspective.zfar;
		}

		EntityManager::add_component<ecs::CameraComponent>(parent, std::move(camera_component));
		auto& nw_camera_component = EntityManager::get_component<ecs::CameraComponent>(parent);

		engine::render::main_camera_index = static_cast<uint32_t>(nw_camera_component.frustumID);
	}

	void Scene::load_light(const cgltf_light* light, const entt::entity parent) {
		if (light->type == cgltf_light_type_directional) {
			ecs::DirectionalLightComponent light_component;
			light_component.color = glm::make_vec3(light->color);
			light_component.intencity = light->intensity / 638.0f;
			EntityManager::add_component<ecs::DirectionalLightComponent>(parent, std::move(light_component));
		}
		else if (light->type == cgltf_light_type_spot) {
			ecs::SpotLightComponent light_component;
			light_component.color = glm::make_vec3(light->color);
			light_component.innerAngle = light->spot_inner_cone_angle;
			light_component.outerAngle = light->spot_outer_cone_angle;
			light_component.intencity = light->intensity / 638.0f;
			EntityManager::add_component<ecs::SpotLightComponent>(parent, std::move(light_component));
		}
		else if (light->type == cgltf_light_type_point) {
			ecs::PointLightComponent light_component;
			light_component.color = glm::make_vec3(light->color);
			light_component.intencity = light->intensity / 638.0f;
			light_component.radius = (std::max)(light->range, 1.0f);
			EntityManager::add_component<ecs::PointLightComponent>(parent, std::move(light_component));
		}
	}

	/*void Scene::LoadAnimations(const tinygltf::Model& root) {
		auto scene_root = EntityManager::get_root();
		auto& root_component = EntityManager::get_component<ecs::RootComponent>(scene_root);

		for (auto& anim : root.animations) {
			ecs::MeshAnimation animation;
			animation.name = anim.name;

			for (auto& samp : anim.samplers) {
				ecs::AnimationSampler sampler{};

				if (samp.interpolation == "LINEAR")
					sampler.interpolation = ecs::EInterpolationType::eLinear;
				if (samp.interpolation == "STEP")
					sampler.interpolation = ecs::EInterpolationType::eStep;
				if (samp.interpolation == "CUBICSPLINE")
					sampler.interpolation = ecs::EInterpolationType::eCubicSpline;

				// Read sampler input time values
				{
					const auto& accessor = root.accessors[samp.input];
					auto stream = get_buffer_stream(root, accessor);

					// TODO: add other type support
					assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

					float* buf = new float[accessor.count];
					edge::utl::copy(buf, stream.data, stream.count * sizeof(float));
					for (uint64_t index = 0; index < stream.count; index++)
						sampler.inputs.push_back(buf[index]);

					delete[] buf;

					for (auto input : sampler.inputs) {
						if (input < animation.start)
							animation.start = input;
						if (input > animation.end)
							animation.end = input;
					}
				}

				// Read sampler output T/R/S values 
				{
					const auto& accessor = root.accessors[samp.output];
					auto stream = get_buffer_stream(root, accessor);

					assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

					if (accessor.type == TINYGLTF_TYPE_VEC3) {
						glm::vec3* buf = new glm::vec3[stream.count];
						edge::utl::copy(buf, stream.data, stream.count * sizeof(glm::vec3));
						for (uint64_t index = 0; index < stream.count; index++)
							sampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));

						delete[] buf;
					}
					else if (accessor.type == TINYGLTF_TYPE_VEC4) {
						glm::vec4* buf = new glm::vec4[stream.count];
						edge::utl::copy(buf, stream.data, stream.count * sizeof(glm::vec4));
						for (uint64_t index = 0; index < stream.count; index++)
							sampler.outputsVec4.push_back(buf[index]);

						delete[] buf;
					}
				}

				animation.samplers.push_back(sampler);
			}

			// Load animation channdls
			for (auto& source : anim.channels) {
				ecs::AnimationChannel channel{};

				if (source.target_path == "rotation")
					channel.path = ecs::EPathType::eRotation;
				if (source.target_path == "translation")
					channel.path = ecs::EPathType::eTranslation;
				if (source.target_path == "scale")
					channel.path = ecs::EPathType::eScale;

				if (source.target_path == "weights") {
					EDGE_LOGW("[Scene] Weights channel is not yet supported. Skipping.");
					continue;
				}
				channel.samplerIndex = source.sampler;

				auto target = node_to_entity.find(source.target_node);
				if (target != node_to_entity.end())
					channel.node = target->second;
				else
					continue;

				animation.channels.push_back(channel);
			}

			root_component.animations.emplace_back(animation);
		}

		EDGE_LOGI("[Scene] Loaded {} animations.", root_component.animations.size());
	}

	void Scene::LoadSkins(const tinygltf::Model& root) {
		auto scene_root = EntityManager::get_root();
		auto& root_component = EntityManager::get_component<ecs::RootComponent>(scene_root);

		for (auto& source : root.skins) {
			ecs::MeshSkin skin{};
			skin.name = source.name;

			// Find skeleton root node
			if (source.skeleton > -1) {
				auto target = node_to_entity.find(source.skeleton);
				if (target != node_to_entity.end())
					skin.root = target->second;
			}

			// Find joint nodes
			for (int jointIndex : source.joints) {
				auto target = node_to_entity.find(jointIndex);
				if (target != node_to_entity.end())
					skin.joints.emplace_back(target->second);
			}

			// Get inverse bind matrices from buffer
			if (source.inverseBindMatrices > -1) {
				const auto& accessor = root.accessors[source.inverseBindMatrices];
				auto stream = get_buffer_stream(root, accessor);
				skin.inverseBindMatrices.resize(stream.count);
				edge::utl::copy(skin.inverseBindMatrices.data(), stream.data, stream.count * sizeof(glm::mat4));
			}

			root_component.skins.emplace_back(skin);
		}

		EDGE_LOGI("[Scene] Loaded {} skins (skeletons).", root_component.skins.size());
	}*/
}