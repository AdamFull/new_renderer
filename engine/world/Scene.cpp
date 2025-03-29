#define TINYGLTF_IMPLEMENTATION
#include "Scene.h"

#include "ecs/components/components.h"
#include "ecs/systems/systems.h"

#include <meshoptimizer.h>

#include "../rendering/Rendering.h"
#include <edge/gfx/gfx_context.h>

#include <edge/util/utl_base.h>
#include <edge/fs/fs_helper.h>
#include <edge/util/Logger.h>
#include <glm/gtc/type_ptr.hpp>

#include <edge/fs/fs_virtual_filesystem.h>
#include <edge/util/utl_profiler.h>

#include "EntityManager.h"

#include <engine/rendering/Globals.h>

namespace engine::world
{
	struct BufferStream {
		const uint8_t* data{ nullptr };
		size_t stride{ 0ull };
		size_t count{ 0ull };
	};

	std::string url_decode(const std::string& str) {
		std::string ret;
		char ch;
		int i, ii, len = str.length();

		for (i = 0; i < len; i++) {
			if (str[i] != '%') {
				if (str[i] == '+')
					ret += ' ';
				else
					ret += str[i];
			}
			else {
				sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
				ch = static_cast<char>(ii);
				ret += ch;
				i = i + 2;
			}
		}
		return ret;
	}

	inline size_t get_size_by_component_type(int32_t component_type) {
		switch (component_type) {
		case TINYGLTF_COMPONENT_TYPE_BYTE:
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: return 1ull; break;
		case TINYGLTF_COMPONENT_TYPE_SHORT:
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return 2ull; break;
		case TINYGLTF_COMPONENT_TYPE_INT:
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
		case TINYGLTF_COMPONENT_TYPE_FLOAT: return 4ull; break;
		case TINYGLTF_COMPONENT_TYPE_DOUBLE: return 8ull; break;
		}

		return 0ull;
	}

	inline size_t get_element_count_by_accessor_type(int32_t type) {
		if (type == TINYGLTF_TYPE_SCALAR)
			return 1ull;
		else if (type == TINYGLTF_TYPE_VEC2)
			return 2ull;
		else if (type == TINYGLTF_TYPE_VEC3)
			return 3ull;
		else if (type == TINYGLTF_TYPE_VEC4)
			return 4ull;
		else if (type == TINYGLTF_TYPE_MAT2)
			return 4ull;
		else if (type == TINYGLTF_TYPE_MAT3)
			return 9ull;
		else if (type == TINYGLTF_TYPE_MAT4)
			return 16ull;

		return 0ull;
	}

	inline size_t get_accessor_element_size(const tinygltf::Accessor& accessor) {
		return get_size_by_component_type(accessor.componentType) * 
			get_element_count_by_accessor_type(accessor.type);
	}

	BufferStream get_buffer_stream(const tinygltf::Model& root, const tinygltf::Accessor& accessor) {
		BufferStream stream;

		const auto& buffer_view = root.bufferViews[accessor.bufferView];
		const auto& buffer = root.buffers[buffer_view.buffer];

		return {
			&buffer.data[accessor.byteOffset + buffer_view.byteOffset],
			get_accessor_element_size(accessor),
			accessor.count
		};
	}

	render::VertexAttributeFlag GetVertexAttributeFlag(std::string_view name) {
		using namespace engine::render;
		if (name.compare("POSITION") == 0) return render::VertexAttributeFlag::ePosition;
		else if (name.compare("COLOR_0") == 0) return render::VertexAttributeFlag::eColor;
		else if (name.compare("TEXCOORD_0") == 0) return render::VertexAttributeFlag::eTexCoord;
		else if (name.compare("NORMAL") == 0) return render::VertexAttributeFlag::eNormal;
		else if (name.compare("TANGENT") == 0) return render::VertexAttributeFlag::eTangent;
		else if (name.compare("JOINTS_0") == 0) return render::VertexAttributeFlag::eJoint;
		else if (name.compare("WEIGHTS_0") == 0) return render::VertexAttributeFlag::eWeight;

		return render::VertexAttributeFlag::ePosition;
	}

	edge::gfx::Format GetVertexFormat(const tinygltf::Accessor& accessor) {
		if (accessor.type == TINYGLTF_TYPE_VEC4) {
			switch (accessor.componentType) {
			case TINYGLTF_COMPONENT_TYPE_BYTE: return edge::gfx::Format::eR8G8B8A8Sint; break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: return edge::gfx::Format::eR8G8B8A8Unorm; break;
			case TINYGLTF_COMPONENT_TYPE_SHORT: return edge::gfx::Format::eR16G16B16A16Sint; break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return edge::gfx::Format::eR16G16B16A16Unorm; break;
			case TINYGLTF_COMPONENT_TYPE_INT: return edge::gfx::Format::eR32G32B32A32Sint; break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: return edge::gfx::Format::eR32G32B32A32Uint; break;
			case TINYGLTF_COMPONENT_TYPE_FLOAT: return edge::gfx::Format::eR32G32B32A32Sfloat; break;
			case TINYGLTF_COMPONENT_TYPE_DOUBLE: return edge::gfx::Format::eR64G64B64A64Sfloat; break;
			default:
				break;
			}
		}
		else if (accessor.type == TINYGLTF_TYPE_VEC3) {
			switch (accessor.componentType) {
			case TINYGLTF_COMPONENT_TYPE_BYTE: return edge::gfx::Format::eR8G8B8Sint; break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: return edge::gfx::Format::eR8G8B8Unorm; break;
			case TINYGLTF_COMPONENT_TYPE_SHORT: return edge::gfx::Format::eR16G16B16Sint; break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return edge::gfx::Format::eR16G16B16Unorm; break;
			case TINYGLTF_COMPONENT_TYPE_INT: return edge::gfx::Format::eR32G32B32Sint; break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: return edge::gfx::Format::eR32G32B32Uint; break;
			case TINYGLTF_COMPONENT_TYPE_FLOAT: return edge::gfx::Format::eR32G32B32Sfloat; break;
			case TINYGLTF_COMPONENT_TYPE_DOUBLE: return edge::gfx::Format::eR64G64B64Sfloat; break;
			default:
				break;
			}
		}
		else if (accessor.type == TINYGLTF_TYPE_VEC2) {
			switch (accessor.componentType) {
			case TINYGLTF_COMPONENT_TYPE_BYTE: return edge::gfx::Format::eR8G8Sint; break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: return edge::gfx::Format::eR8G8Unorm; break;
			case TINYGLTF_COMPONENT_TYPE_SHORT: return edge::gfx::Format::eR16G16Sint; break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return edge::gfx::Format::eR16G16Unorm; break;
			case TINYGLTF_COMPONENT_TYPE_INT: return edge::gfx::Format::eR32G32Sint; break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: return edge::gfx::Format::eR32G32Uint; break;
			case TINYGLTF_COMPONENT_TYPE_FLOAT: return edge::gfx::Format::eR32G32Sfloat; break;
			case TINYGLTF_COMPONENT_TYPE_DOUBLE: return edge::gfx::Format::eR64G64Sfloat; break;
			default:
				break;
			}
		}
		else if (accessor.type == TINYGLTF_TYPE_SCALAR) {
			switch (accessor.componentType) {
			case TINYGLTF_COMPONENT_TYPE_BYTE: return edge::gfx::Format::eR8Sint; break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: return edge::gfx::Format::eR8Unorm; break;
			case TINYGLTF_COMPONENT_TYPE_SHORT: return edge::gfx::Format::eR16Sint; break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return edge::gfx::Format::eR16Unorm; break;
			case TINYGLTF_COMPONENT_TYPE_INT: return edge::gfx::Format::eR32Sint; break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: return edge::gfx::Format::eR32Uint; break;
			case TINYGLTF_COMPONENT_TYPE_FLOAT: return edge::gfx::Format::eR32Sfloat; break;
			case TINYGLTF_COMPONENT_TYPE_DOUBLE: return edge::gfx::Format::eR64Sfloat; break;
			default:
				break;
			}
		}

		return edge::gfx::Format::eUndefined;
	}

	edge::gfx::IndexType GetIndexType(const tinygltf::Accessor& accessor)
	{
		switch (accessor.componentType)
		{
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return edge::gfx::IndexType::eUint16; break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: return edge::gfx::IndexType::eUint32; break;
		default:
			break;
		}

		return edge::gfx::IndexType::eUint32;
	}

	render::GeometryBufferCreateInfo GetGeometryBufferCreateInfo(const tinygltf::Model& root, const tinygltf::Primitive& primitive) {
		render::GeometryBufferCreateInfo create_info{};

		for (const auto& [name, accessor_index] : primitive.attributes) {
			// NOT SUPPORTED FOR NOW
			if (name == "TEXCOORD_1")
				continue;

			const auto& accessor = root.accessors[accessor_index];

			auto& new_description = create_info.attribute_descriptions.emplace_back();
			new_description.attribute_flag = GetVertexAttributeFlag(name);
			new_description.format = GetVertexFormat(accessor);
		}

		if (primitive.indices >= 0) {
			const auto& accessor = root.accessors[primitive.indices];
			create_info.index_description.type = GetIndexType(accessor);
		}

		std::sort(create_info.attribute_descriptions.begin(), create_info.attribute_descriptions.end(),
			[](const auto& lhs, const auto& rhs) {
				return edge::enum_cast(lhs.attribute_flag) < edge::enum_cast(rhs.attribute_flag);
			});

		return create_info;
	}

	// FIXME: need to set correct filter mode
	edge::gfx::Filter GetSamplerFilter(int32_t filter) {
		edge::gfx::Filter gfx_filter{ edge::gfx::Filter::eNearest };

		switch (filter) {
		case TINYGLTF_TEXTURE_FILTER_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
			gfx_filter = edge::gfx::Filter::eNearest;
			break;
		case TINYGLTF_TEXTURE_FILTER_LINEAR:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
			gfx_filter = edge::gfx::Filter::eLinear;
			break;
		default:
			break;
		}

		return gfx_filter;
	}

	edge::gfx::SamplerAddressMode GetSamplerAddressMode(int32_t address_mode) {
		edge::gfx::SamplerAddressMode gfx_address_mode{};

		switch (address_mode) {
		case TINYGLTF_TEXTURE_WRAP_REPEAT:
			gfx_address_mode = edge::gfx::SamplerAddressMode::eRepeat;
			break;
		case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
			gfx_address_mode = edge::gfx::SamplerAddressMode::eClampToEdge;
			break;
		case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
			gfx_address_mode = edge::gfx::SamplerAddressMode::eMirror;
			break;
		default:
			break;
		}

		return gfx_address_mode;
	}

	edge::gfx::SamplerCreateInfo GetSamplerCreateInfo(const tinygltf::Sampler& sampler) {
		edge::gfx::SamplerCreateInfo create_info{};

		create_info.min_filter = GetSamplerFilter(sampler.minFilter);
		create_info.mag_filter = GetSamplerFilter(sampler.magFilter);
		create_info.mip_map_mode = GetSamplerFilter(sampler.minFilter);

		create_info.address_u = GetSamplerAddressMode(sampler.wrapS);
		create_info.address_v = GetSamplerAddressMode(sampler.wrapT);

		create_info.max_anisotropy = 16.f;

		return create_info;
	}

	Scene::Scene(render::IRenderer& renderer) :
		renderer{ renderer } {
		EntityManager::RegisterSystem<ecs::CInputSystem>();
		EntityManager::RegisterSystem<ecs::CAnimationSystem>();
		EntityManager::RegisterSystem<ecs::CScenegraphSystem>(renderer);
		EntityManager::RegisterSystem<ecs::CSkinningSystem>(renderer);

		EntityManager::RegisterSystem<ecs::CameraSystem>(renderer);
		EntityManager::RegisterSystem<ecs::CShadowsSystem>(renderer);
		EntityManager::RegisterSystem<ecs::LightingSystem>(renderer);
		EntityManager::RegisterSystem<ecs::CMeshSystem>(renderer);
	}

	Scene::~Scene() {
	}

	void Scene::Load(const std::string& path) {
		tinygltf::FsCallbacks gltf_fs_callbacks;
		gltf_fs_callbacks.user_data = this;
		gltf_fs_callbacks.FileExists = [](const std::string& filename, void* userdata) -> bool {
			return edge::fs::exists(filename);
			};
		gltf_fs_callbacks.ExpandFilePath = [](const std::string& filename, void* userdata) -> std::string {
			std::array<char, 1024ull> buffer{};
			return std::string(edge::fs::to_posix_path(filename, buffer));
			};
		gltf_fs_callbacks.GetFileSizeInBytes = [](size_t* filesize_out, std::string* err,
			const std::string& filename, void* userdata) -> bool {
				*filesize_out = edge::fs::file_size(filename);
				return true;
			};
		gltf_fs_callbacks.ReadWholeFile = [](std::vector<uint8_t>* u8data, std::string* err,
			const std::string& filename, void* userdata) -> bool {
				return edge::fs::read_file(filename, *u8data);
			};
		gltf_fs_callbacks.WriteWholeFile = [](std::string* err, const std::string& filename,
			const std::vector<unsigned char>& u8data, void* userdata) -> bool {
				return edge::fs::write_file(filename, u8data);
			};

		tinygltf::TinyGLTF gltf_context;
		gltf_context.SetFsCallbacks(gltf_fs_callbacks);
		// TODO: Implement this function. Will be needed for glb format.
		gltf_context.SetImageLoader([](tinygltf::Image* image, const int imageIndex,
			std::string* err, std::string* warn, int req_width, int req_height,
			const unsigned char* bytes, int size, void* userdata) -> bool {
				return true;//tinygltf::LoadImageData(image, imageIndex, error, warning, req_width, req_height, bytes, size, userData);
			}, this);

		tinygltf::Model gltf_root;
		std::string error, warning;
		if (!gltf_context.LoadASCIIFromFile(&gltf_root, &error, &warning, path)) {
			EDGE_LOGE("[Scene] Failed to load scene file \"{}\" with error: {}", path, error);
			return;
		}

		if (!warning.empty()) {
			EDGE_LOGW("[Scene] {}", warning);
		}

		scene_root_directory = edge::fs::parent_path(path);

		LoadGLTF(gltf_root);
	}

	void Scene::Update(float delta_time)
	{
		EDGE_PROFILE_ZONE_SCOPED;
		EntityManager::Update(delta_time);
	}

	void Scene::LoadGLTF(tinygltf::Model& root) {
		// Load textures
		for (const auto& texture : root.textures) {
			int32_t texture_source{ texture.source };
			if (texture.source < 0) {
				EDGE_LOGE("[Scene] Loading basisu textures is currently unavaliable.");
				continue;
			}

			auto& image = root.images[texture_source];
			auto& sampler = root.samplers[texture.sampler];

			// TODO: Use sampler cache
			auto sampler_create_info = GetSamplerCreateInfo(sampler);
			edge::gfx::ISampler* texture_sampler = edge::gfx::create_sampler(sampler_create_info);

			// TODO: add ability to override format and sampler params
			// TODO: find format from materials

			std::array<char, 1024ull> buffer{};
			auto image_path = edge::fs::path_append(scene_root_directory, url_decode(image.uri), std::span(buffer));
			resources.textures.push_back(renderer.AddRenderResource(image_path, texture_sampler));
		}

		for (uint64_t idx = 0ull; idx < root.materials.size(); ++idx) {
			auto& material = root.materials[idx];
			shlib::Material render_material{};

			// TODO: add my own ext that will contain shader EXT_material_shader or something. But in my case it will contain name in format {library}:{shader_name} and other helpfull data

			// Attach base color (aka albedo)
			if (auto& texture = material.pbrMetallicRoughness.baseColorTexture; texture.index >= 0)
				render_material.baseColorTexture = resources.textures[texture.index];

			// Attach metallic roughness texture (metallic = green, roughness = blue)
			if (auto& texture = material.pbrMetallicRoughness.metallicRoughnessTexture; texture.index >= 0)
				render_material.metallicRoughnessTexture = resources.textures[texture.index];

			// Attach normal map texture
			if (auto& texture = material.normalTexture; texture.index >= 0)
			{
				render_material.normalScale = static_cast<f32>(texture.scale);
				render_material.normalTexture = resources.textures[texture.index];
			}

			// Attach baked ambient occlusion texture
			if (auto& texture = material.occlusionTexture; texture.index >= 0)
			{
				render_material.occlusionStrenth = static_cast<f32>(texture.strength);
				render_material.occlusionTexture = resources.textures[texture.index];
			}

			// Attach emissive texture
			if (auto& texture = material.emissiveTexture; texture.index >= 0)
				render_material.emissiveTexture = resources.textures[texture.index];

			render_material.emissiveFactor = glm::make_vec3(material.emissiveFactor.data());
			render_material.roughnessFactor = static_cast<f32>(material.pbrMetallicRoughness.roughnessFactor);
			render_material.metallicFactor = static_cast<f32>(material.pbrMetallicRoughness.metallicFactor);
			render_material.baseColorFactor = glm::make_vec4(material.pbrMetallicRoughness.baseColorFactor.data());

			if (material.alphaMode.compare("BLEND") == 0)
				render_material.alphaMode = 0;
			else if (material.alphaMode.compare("MASK") == 0)
				render_material.alphaMode = 1;
			else
				render_material.alphaMode = 2;

			render_material.alphaCutoff = static_cast<f32>(material.alphaCutoff);
			render_material.doubleSided = material.doubleSided;

			renderer.AddMaterial(render_material);
		}

		// Loading nodes
		const auto& scene = root.scenes[root.defaultScene > -1 ? root.defaultScene : 0];
		auto scene_root = EntityManager::CreateRoot(scene.name);

		for (const auto& node_index : scene.nodes)
			LoadNode(root, scene_root, node_index);

		LoadAnimations(root);
		LoadSkins(root);

		EntityManager::Create();
	}

	void Scene::LoadNode(const tinygltf::Model& root, const entt::entity parent, uint32_t node_index, f32 global_scale) {
		const auto& node = root.nodes[node_index];

		auto new_node = EntityManager::CreateNode(node.name);

		EDGE_LOGI("[Scene] Loadind node: {}:{}.", node.name, node_index);

		node_to_entity[node_index] = new_node;

		// Parse node local transform
		auto& transform = EntityManager::GetComponent<ecs::FTransformComponent>(new_node);

		if (!node.translation.empty()) {
			transform.lposition = glm::make_vec3(node.translation.data());
		}
		if (!node.rotation.empty()) {
			transform.lrotation = glm::make_quat(node.rotation.data());
		}
		if (!node.scale.empty()) {
			transform.lscale = glm::make_vec3(node.scale.data());
		}
		if (!node.matrix.empty()) {
			transform.lmodel = glm::make_mat4(node.matrix.data());
		}

		// Parse node children
		if (!node.children.empty()) {
			for (const auto& child_node_index : node.children)
				LoadNode(root, node_to_entity[node_index], child_node_index);
		}

		if (node.mesh >= 0)
			LoadMeshComponent(root, node_to_entity[node_index], node_index);

		if (node.camera >= 0)
			LoadCameraComponent(root, node_to_entity[node_index], node_index);

		if (node.light >= 0)
			LoadLightComponent(root, node_to_entity[node_index], node_index);

		EntityManager::AttachChild(parent, new_node);
	}

	// Calculate whole mesh bounds and encoders
	void ComputeMeshBounds(const tinygltf::Model& root, const tinygltf::Mesh& gltf_mesh, shlib::Mesh& mesh) {
		shlib::BoundingBox bb{};
		mesh.bb_enc = glm::vec3(1.0f);

		for (const auto& gltf_primitive : gltf_mesh.primitives) {
			// Iterate whole all accessors
			for (const auto& [name, accessor_index] : gltf_primitive.attributes) {
				// NOT SUPPORTED FOR NOW
				if (name == "TEXCOORD_1")
					continue;

				const auto& accessor = root.accessors[accessor_index];
				auto stream = get_buffer_stream(root, accessor);

				auto attribute_type = GetVertexAttributeFlag(name);
				if (attribute_type & render::VertexAttributeFlag::ePosition) {
					const glm::vec3* begin = reinterpret_cast<const glm::vec3*>(stream.data);
					const glm::vec3* end = begin + stream.count;

					for (auto it = begin; it != end; ++it)
					{
						mesh.bb_enc = glm::max(mesh.bb_enc, glm::abs(*it));
						bb.min = glm::min(bb.min, *it);
						bb.max = glm::max(bb.max, *it);
					}
				}
				else if (attribute_type & render::VertexAttributeFlag::eTexCoord) {
					const glm::vec2* begin = reinterpret_cast<const glm::vec2*>(stream.data);
					const glm::vec2* end = begin + stream.count;

					for (auto it = begin; it != end; ++it)
					{
						mesh.uv_enc.min = glm::min(mesh.uv_enc.min, *it);
						mesh.uv_enc.max = glm::max(mesh.uv_enc.max, *it);
					}
				}
				else
					continue;
			}
		}

		mesh.bb_enc = glm::exp2(glm::ceil(glm::log2(mesh.bb_enc)));

		// Round off the minimum and the maximum of the bounding square to the next
		// power of 2 if they are not equal to 0
		{
			// Select a relatively big default square size to avoid log2 NaN
			glm::vec2 p = glm::log2(glm::max(glm::max(-mesh.uv_enc.min, mesh.uv_enc.max), 1.0f));
			glm::vec2 bs = glm::exp2(glm::ceil(p));
			mesh.uv_enc.min = glm::mix(glm::vec2(0.0f), -bs, glm::notEqual(mesh.uv_enc.min, glm::vec2(0.0f)));
			mesh.uv_enc.max = glm::mix(glm::vec2(0.0f), bs, glm::notEqual(mesh.uv_enc.max, glm::vec2(0.0f)));
		}

		mesh.ebb = shlib::EncodeBoundingBox(bb, mesh.bb_enc);
	}

	// Maximum vertex size
	// position: vec3 - 12 bytes -> i16vec2 - 6 bytes
	// texcoord: vec2 - 8 bytes -> u16vec2 - 4 bytes
	// normal: vec3 - 12 bytes -> u16vec2 - 4 bytes
	// tangent: vec4 - 16 bytes -> u16vec2 - 4 bytes
	// color: vec4 - 16 bytes -> u8vec4 - 4 bytes
	// joints: u16vec4/u8vec4 - 8 bytes/4 bytes
	// weights: vec4 - 16 bytes -> vec3 - 12 bytes -> u16vec3 - 6 bytes
	// Total(before): 12 + 8 + 12 + 16 + 16 + 8|4 + 16 = 84|88
	// Total(after): 12 + 4 + 4 + 4 + 4 + 8|4 + 6 = 32|36

	struct VertexData {
		std::vector<glm::i16vec3> positions;
		std::vector<glm::u8vec4> colors;
		std::vector<glm::u16vec2> texcoords;
		std::vector<glm::u16vec2> normals;
		std::vector<glm::u16vec2> tangents;
		// TODO: i can use uint16_t for joints by default
		std::vector<uint8_t> joints;
		std::vector<glm::u16vec3> weights;
	};

	template<typename _From, typename _To, typename _Pred>
	std::span<const uint8_t> Transform(const tinygltf::Model& root, const tinygltf::Accessor& accessor, std::vector<_To>& out, _Pred&& pred) {
		auto stream = get_buffer_stream(root, accessor);
		assert(stream.stride == sizeof(_From) && "Incorrect _From data type size.");

		auto* begin = reinterpret_cast<const _From*>(stream.data);
		auto* end = begin + stream.count;

		for (auto it = begin; it != end; ++it)
			out.emplace_back(pred(*it));

		auto* bytes_begin = reinterpret_cast<const uint8_t*>(out.data());
		size_t bytes_count = out.size() * sizeof(_To);

		return std::span<const uint8_t>(bytes_begin, bytes_count);
	}

	void Scene::LoadMeshComponent(const tinygltf::Model& root, const entt::entity parent, uint32_t node_index) {
		auto& render_world = renderer.GetRenderWorld();

		const auto& gltf_node = root.nodes[node_index];
		const auto& gltf_mesh = root.meshes[gltf_node.mesh];

		uint32_t joint_count{ 0u };
		if (gltf_node.skin != -1) {
			const auto& skin = root.skins[gltf_node.skin];
			joint_count = skin.joints.size();
		}

		// Create gpu mesh instance 
		auto [mesh_id, mesh] = render_world.CreateMesh(joint_count);
		ComputeMeshBounds(root, gltf_mesh, mesh);

		auto& mesh_component = EntityManager::AddComponent<ecs::FMeshComponent>(parent, ecs::FMeshComponent{});
		mesh_component.mesh = mesh_id;
		mesh_component.skin = gltf_node.skin;

		// Create encode matrices for mesh
		glm::mat3 position_encode_mat = shlib::MakeEncodePositionMatrix(mesh.bb_enc);
		glm::mat3 normal_encode_mat = glm::transpose(glm::inverse(position_encode_mat));

		// Load mesh primitives
		for (uint64_t i = 0ull; i < gltf_mesh.primitives.size(); ++i) {
			const auto& primitive = gltf_mesh.primitives[i];

			// Load vertices
			render::GeometryBufferCreateInfo geometry_ci = GetGeometryBufferCreateInfo(root, primitive);
			auto geometry_buffer_id = renderer.RequestDeinterleavedGeometryBuffer(geometry_ci);
			auto& geometry_buffer = renderer.LookUpGeometryBuffer(geometry_buffer_id);

			shlib::BoundingBox bb{};
			VertexData vertex_data{};

			uint32_t stream_count{ 0u };
			std::array<render::AttributeStream, render::k_max_attribure_description> attribute_streams{};
			for (const auto& [name, accessor_index] : primitive.attributes) {
				// NOT SUPPORTED FOR NOW
				if (name == "TEXCOORD_1")
					continue;

				const auto& accessor = root.accessors[accessor_index];
				auto attribute_type = GetVertexAttributeFlag(name);

				// Create new data stream
				auto& attrib_stream = attribute_streams[stream_count++];
				attrib_stream.attribute_flag = attribute_type;
				attrib_stream.count = accessor.count;

				if (attribute_type & render::VertexAttributeFlag::ePosition) {
					attrib_stream.byte_range = Transform<glm::vec3, glm::i16vec3>(root, accessor, vertex_data.positions,
						[&mesh](const glm::vec3& in)
						{  return shlib::EncodePosition(in, mesh.bb_enc); });

					// Calculate bounding box for this mesh primitive
					auto stream = get_buffer_stream(root, accessor);
					auto* begin = reinterpret_cast<const glm::vec3*>(stream.data);
					auto* end = begin + stream.count;

					for (auto it = begin; it != end; ++it)
					{
						bb.min = glm::min(bb.min, *it);
						bb.max = glm::max(bb.max, *it);
					}
				}
				else if (attribute_type & render::VertexAttributeFlag::eColor) {
					attrib_stream.byte_range = Transform<glm::vec4, glm::u8vec4>(root, accessor, vertex_data.colors,
						[](const glm::vec4& in) { return shlib::EncodeColor(in); });
				}
				else if (attribute_type & render::VertexAttributeFlag::eTexCoord) {
					attrib_stream.byte_range = Transform<glm::vec2, glm::u16vec2>(root, accessor, vertex_data.texcoords,
						[&mesh](const glm::vec2& in) { return shlib::EncodeTexcoord(in, mesh.uv_enc); });
				}
				else if (attribute_type & render::VertexAttributeFlag::eNormal) {
					attrib_stream.byte_range = Transform<glm::vec3, glm::u16vec2>(root, accessor, vertex_data.normals,
						[&normal_encode_mat](const glm::vec3& in)
						{ return shlib::EncodeNormal(glm::normalize(normal_encode_mat * in)); });
				}
				else if (attribute_type & render::VertexAttributeFlag::eTangent) {
					attrib_stream.byte_range = Transform<glm::vec4, glm::u16vec2>(root, accessor, vertex_data.tangents,
						[&position_encode_mat](const glm::vec4& in)
						{
							return shlib::EncodeTangent(glm::vec4(glm::normalize(position_encode_mat * in), in.w));
						});
				}
				// TODO: can be compressed with using u16vec3
				else if (attribute_type & render::VertexAttributeFlag::eWeight) {
					attrib_stream.byte_range = Transform<glm::vec4, glm::u16vec3>(root, accessor, vertex_data.weights,
						[](const glm::vec4& in) { return shlib::EncodeWeights(in); });
				}
				else {
					// Just copy, transcoding not supported yet
					auto stream = get_buffer_stream(root, accessor);
					attrib_stream.byte_range = std::span<const uint8_t>(stream.data, stream.count * stream.stride);
					attrib_stream.count = stream.count;
				}
			}

			render::IndexStream index_stream{};
			if (primitive.indices >= 0) {
				const auto& accessor = root.accessors[primitive.indices];
				auto stream = get_buffer_stream(root, accessor);

				index_stream.byte_range = std::span<const uint8_t>(stream.data, stream.count * stream.stride);
				index_stream.count = stream.count;
			}

			auto vertex_range = geometry_buffer.AddMesh(attribute_streams, index_stream);

			shlib::SubMesh submesh{};
			submesh.first_index = vertex_range.first_index;
			submesh.index_count = vertex_range.index_count;
			submesh.first_vertex = vertex_range.first_vertex;
			submesh.vertex_count = vertex_range.vertex_count;
			submesh.material = primitive.material;
			submesh.ebb = shlib::EncodeBoundingBox(bb, mesh.bb_enc);

			render_world.AttachSubmesh(mesh_id, submesh);
		}
	}

	void Scene::LoadCameraComponent(const tinygltf::Model& root, const entt::entity parent, uint32_t node_index) {
		const auto& node = root.nodes[node_index];
		const auto& camera = root.cameras[node.camera];

		ecs::FCameraComponent camera_component;
		if (camera.type.compare("orthographic") == 0) {
			camera_component.ctype = ecs::ECameraType::eOrthographic;
			camera_component.xmag = camera.orthographic.xmag;
			camera_component.ymag = camera.orthographic.ymag;
			camera_component.nearPlane = camera.orthographic.znear;
			camera_component.farPlane = camera.orthographic.zfar;
		}
		else if (camera.type.compare("perspective") == 0) {
			camera_component.ctype = ecs::ECameraType::ePerspective;
			camera_component.aspectRatio = camera.perspective.aspectRatio;
			camera_component.fieldOfView = glm::degrees(2.0f * glm::atan(glm::tan(camera.perspective.yfov * 0.5f) * camera.perspective.aspectRatio));
			camera_component.nearPlane = camera.perspective.znear;
			camera_component.farPlane = camera.perspective.zfar;
		}

		EntityManager::AddComponent<ecs::FCameraComponent>(parent, std::move(camera_component));
		auto& nw_camera_component = EntityManager::GetComponent<ecs::FCameraComponent>(parent);

		engine::render::main_camera_index = nw_camera_component.frustumID;
	}

	void Scene::LoadLightComponent(const tinygltf::Model& root, const entt::entity parent, uint32_t node_index) {
		const auto& node = root.nodes[node_index];
		
		auto& light = root.lights[node.light];
		if (light.type.compare("directional") == 0) {
			ecs::FDirectionalLightComponent light_component;
			light_component.color = glm::make_vec3(light.color.data());
			light_component.intencity = light.intensity / 638.0;
			EntityManager::AddComponent<ecs::FDirectionalLightComponent>(parent, std::move(light_component));
		}
		else if (light.type.compare("spot") == 0) {
			ecs::FSpotLightComponent light_component;
			light_component.color = glm::make_vec3(light.color.data());
			light_component.innerAngle = light.spot.innerConeAngle;
			light_component.outerAngle = light.spot.outerConeAngle;
			light_component.intencity = light.intensity / 638.0;
			EntityManager::AddComponent<ecs::FSpotLightComponent>(parent, std::move(light_component));
		}
		else if (light.type.compare("point")) {
			ecs::FPointLightComponent light_component;
			light_component.color = glm::make_vec3(light.color.data());
			light_component.intencity = light.intensity / 638.0;
			light_component.radius = (std::max)(light.range, 1.0);
			EntityManager::AddComponent<ecs::FPointLightComponent>(parent, std::move(light_component));
		}
	}

	void Scene::LoadAnimations(const tinygltf::Model& root) {
		auto scene_root = EntityManager::GetRoot();
		auto& root_component = EntityManager::GetComponent<ecs::FRootComponent>(scene_root);

		for (auto& anim : root.animations) {
			ecs::FMeshAnimation animation;
			animation.name = anim.name;

			for (auto& samp : anim.samplers) {
				ecs::FAnimationSampler sampler{};

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

					f32* buf = new f32[accessor.count];
					util::copy(buf, stream.data, stream.count * sizeof(f32));
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
						util::copy(buf, stream.data, stream.count * sizeof(glm::vec3));
						for (uint64_t index = 0; index < stream.count; index++)
							sampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));

						delete[] buf;
					}
					else if (accessor.type == TINYGLTF_TYPE_VEC4) {
						glm::vec4* buf = new glm::vec4[stream.count];
						util::copy(buf, stream.data, stream.count * sizeof(glm::vec4));
						for (uint64_t index = 0; index < stream.count; index++)
							sampler.outputsVec4.push_back(buf[index]);

						delete[] buf;
					}
				}

				animation.samplers.push_back(sampler);
			}

			// Load animation channdls
			for (auto& source : anim.channels) {
				ecs::FAnimationChannel channel{};

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
		auto scene_root = EntityManager::GetRoot();
		auto& root_component = EntityManager::GetComponent<ecs::FRootComponent>(scene_root);

		for (auto& source : root.skins) {
			ecs::FMeshSkin skin{};
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
				util::copy(skin.inverseBindMatrices.data(), stream.data, stream.count * sizeof(glm::mat4));
			}

			root_component.skins.emplace_back(skin);
		}

		EDGE_LOGI("[Scene] Loaded {} skins (skeletons).", root_component.skins.size());
	}
}