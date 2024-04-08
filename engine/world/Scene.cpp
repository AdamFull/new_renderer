#include "Scene.h"

#include <engine/world/SceneGraph.h>
#include <engine/world/ecs/components/components.h>
#include <engine/world/ecs/systems/systems.h>

#include<easygltf/easygltf_loader.h>
#include<easygltf/easygltf_helpers.h>

#include <meshoptimizer.h>

#include <engine/filesystem/vfs_helper.h>
#include <logger/logger.h>
#include <glm/gtc/type_ptr.hpp>

#include <engine/profiler/Profiler.h>

#include "TerrainGenerator.h"

using namespace engine::world;
using namespace engine::world::ecs;
using namespace engine::filesystem;

// Random fill
template<class _Ty, size_t _Size>
void random_fill(std::array<_Ty, _Size>& out, const _Ty& min, const _Ty& max)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<_Ty> dis(min, max);

	for (auto& element : out)
		element = dis(gen);
}

engine::render::EVertexAttributeType get_vertex_attribute_type_by_name(const stl::string& name)
{
	using namespace engine::render;

	if (name == "POSITION") return EVertexAttributeType::ePosition;
	else if (name == "COLOR_0") return EVertexAttributeType::eColor;
	else if (name == "TEXCOORD_0") return EVertexAttributeType::eTexCoord;
	else if (name == "NORMAL") return EVertexAttributeType::eNormal;
	else if (name == "TANGENT") return EVertexAttributeType::eTangent;
	else if (name == "JOINTS_0") return EVertexAttributeType::eJoint;
	else if (name == "WEIGHTS_0") return EVertexAttributeType::eWeight;
}

class FSWrapper : public easygltf::IFilesystemWrapper
{
public:
	FSWrapper(IVirtualFileSystemInterface* vfs) : m_pVFS(vfs) {}
	~FSWrapper() override = default;

	bool exists(const stl::string& path) override
	{
		return m_pVFS->exists(path);
	}

	stl::string expandFilePath(const stl::string& path) override
	{
		return fs::normalize(fs::to_posix_path(path));
	}

	bool readFile(const stl::string& path, stl::vector<uint8_t>& data) override
	{
		return m_pVFS->readFile(path, data);
	}

	bool writeFile(const stl::string& path, const stl::vector<uint8_t>& data) override
	{
		return m_pVFS->writeFile(path, data);
	}

	u64 fileSize(const stl::string& path) override
	{
		return m_pVFS->fileSize(path);
	}
private:
	IVirtualFileSystemInterface* m_pVFS{ nullptr };
};

void convert_indices_space(stl::vector<uint8_t>& indices, u64 index_size, i32 vertex_start)
{
	for (u64 shift = 0ull; shift < indices.size() / index_size; shift += index_size)
	{
		switch (index_size)
		{
		case 4: {
			u32 index = *reinterpret_cast<u32*>(&indices[shift]);
			index += vertex_start;
			*reinterpret_cast<u32*>(&indices[shift]) = index;
		} break;
		case 2:
		{
			uint16_t index = *reinterpret_cast<uint16_t*>(&indices[shift]);
			index += vertex_start;
			*reinterpret_cast<uint16_t*>(&indices[shift]) = utl::cast<uint16_t>(index);
		} break;
		case 1:
		{
			uint8_t index = indices[shift];
			index += vertex_start;
			indices[shift] = index;
		} break;
		default:
			throw stl::runtime_error("Unsupported vertex size");
		}
	}
}

CScene::CScene(IVirtualFileSystemInterface* vfs, render::IRenderInterface* render_interface)
{
	m_pVFS = vfs;
	m_pRenderInterface = render_interface;

	m_pSceneGraph = new CSceneGraph();

	m_systems.emplace_back(stl::make_unique<ecs::CInputSystem>(m_pSceneGraph));
	m_systems.emplace_back(stl::make_unique<ecs::CAnimationSystem>(m_pSceneGraph));
	m_systems.emplace_back(stl::make_unique<ecs::CScenegraphSystem>(m_pSceneGraph));
	//m_systems.emplace_back(stl::make_unique<ecs::CAnimationSystem>(m_pSceneGraph));
	m_systems.emplace_back(stl::make_unique<ecs::CSkinningSystem>(m_pSceneGraph, render_interface));

	m_systems.emplace_back(stl::make_unique<ecs::CCameraSystem>(m_pSceneGraph, render_interface));
	m_systems.emplace_back(stl::make_unique<ecs::CShadowsSystem>(m_pSceneGraph, render_interface));
	m_systems.emplace_back(stl::make_unique<ecs::CLightingSystem>(m_pSceneGraph, render_interface));
	m_systems.emplace_back(stl::make_unique<ecs::CMeshSystem>(m_pSceneGraph, render_interface));
}

CScene::~CScene()
{
	if (m_pSceneGraph)
	{
		delete m_pSceneGraph;
		m_pSceneGraph = nullptr;
	}
}

void CScene::load(const stl::string& path)
{
	easygltf::CGLTFLoader loader{};
	loader.setFilesystemWrapper(stl::make_unique<FSWrapper>(m_pVFS));
	
	easygltf::FGLTFRoot root{};
	if (!loader.load(path, root))
		return;

	loadGLTF(root);
}

void CScene::makeGenerateScene(const stl::string& path)
{
	easygltf::CGLTFLoader loader{};
	loader.setFilesystemWrapper(stl::make_unique<FSWrapper>(m_pVFS));

	easygltf::FGLTFRoot root{};
	if (!loader.load(path, root))
		return;

	if (root.meshes.empty())
	{
		log_error("[Scene] No meshes to duplicate found.");
		return;
	}

	auto& current_scene = root.scenes.at(root.scene < 0 ? 0 : root.scene);

	constexpr const u64 duplicate_count = 60000;
	for (u64 idx = 0ull; idx < duplicate_count; ++idx)
	{
		// Add new node to scene
		current_scene.nodes.emplace_back(static_cast<int32_t>(root.nodes.size()));
	
		auto& new_node = root.nodes.emplace_back();
		new_node.name = std::format("Node {}", idx);
		new_node.mesh = root.meshes.size() - 1ull;
		random_fill(new_node.translation, -35.0, 35.0);
		random_fill(new_node.rotation, 0.0, 1.0);
		random_fill(new_node.scale, 0.1, 0.5);
	
		auto& new_animation = root.animations.emplace_back();
		new_animation = root.animations.at(0);
		
		for (auto& channel : new_animation.channels)
			channel.target.node = current_scene.nodes.back();
	}

	loadGLTF(root);
}

void CScene::update(f32 dt)
{
	TRACY_PROFILE_FUNCTION();

	TRACY_PROFILE_SCOPE("Scene update")

	for (auto& system : m_systems)
		system->update(dt);
}

void CScene::loadGLTF(easygltf::FGLTFRoot& root)
{
	// Load textures
	for (const auto& texture : root.textures)
	{
		i32 texture_source{ texture.source };
		if (texture.source < 0)
			texture_source = texture.khrTextureBasisU.source;

		auto& image = root.images.at(texture_source);
		auto& sampler = root.images.at(texture.sampler);

		// TODO: add ability to override format and sampler params
		// TODO: find format from materials
		auto image_path = fs::path_append(root.rootDirectory, easygltf::url_decode(image.uri));
		m_resources.textures.emplace_back(m_pRenderInterface->createTexture(image_path));
		m_resources.lastTexture++;
	}

	// Load materials
	auto& render_world = m_pRenderInterface->getRenderWorld();
	auto& render_packet = m_pRenderInterface->getRenderPacket();

	stl::vector<render::FMaterial> materials{};
	for (u64 idx = 0ull; idx < root.materials.size(); ++idx)
	{
		auto& material = root.materials[idx];
		auto& material_data = materials.emplace_back();

		m_temporary.materialNames.emplace_back(material.name);
		// TODO: add my own ext that will contain shader EXT_material_shader or something. But in my case it will contain name in format {library}:{shader_name} and other helpfull data

		// Attach base color (aka albedo)
		if (auto& texture = material.pbrMetallicRoughness.baseColorTexture; texture.index >= 0)
			material_data.baseColorTexture = m_resources.textures.at(texture.index);

		// Attach metallic roughness texture (metallic = green, roughness = blue)
		if (auto& texture = material.pbrMetallicRoughness.metallicRoughnessTexture; texture.index >= 0)
			material_data.metallicRoughnessTexture = m_resources.textures.at(texture.index);

		// Attach normal map texture
		if (auto& texture = material.normalTexture; texture.index >= 0)
		{
			material_data.normalScale = utl::cast<f32>(texture.scale);
			material_data.normalTexture = m_resources.textures.at(texture.index);
		}

		// Attach baked ambient occlusion texture
		if (auto& texture = material.occlusionTexture; texture.index >= 0)
		{
			material_data.occlusionStrenth = utl::cast<f32>(texture.strength);
			material_data.occlusionTexture = m_resources.textures.at(texture.index);
		}

		// Attach emissive texture
		if (auto& texture = material.emissiveTexture; texture.index >= 0)
			material_data.emissiveTexture = m_resources.textures.at(texture.index);

		material_data.emissiveFactor = glm::make_vec3(material.emissiveFactor.data());
		material_data.roughnessFactor = utl::cast<f32>(material.pbrMetallicRoughness.roughnessFactor);
		material_data.metallicFactor = utl::cast<f32>(material.pbrMetallicRoughness.metallicFactor);
		material_data.baseColorFactor = glm::make_vec4(material.pbrMetallicRoughness.baseColorFactor.data());

		if (material.alphaMode == easygltf::base::enumerators::alpha_mode::blend)
			material_data.alphaMode = utl::cast<u32>(render::EAlphaMode::eBlend);
		else if (material.alphaMode == easygltf::base::enumerators::alpha_mode::mask)
			material_data.alphaMode = utl::cast<u32>(render::EAlphaMode::eMask);
		else
			material_data.alphaMode = utl::cast<u32>(render::EAlphaMode::eOpaque);

		material_data.alphaCutoff = utl::cast<f32>(material.alphaCutoff);
		material_data.doubleSided = material.doubleSided;

		render_packet->stats.materialCount++;
	}

	render_world->addMaterials(materials);

	// Loading nodes
	const easygltf::Scene& scene = root.scenes[root.scene > -1 ? root.scene : 0];
	auto root_node = m_pSceneGraph->createRoot(scene.name);

	for (const auto& node_index : scene.nodes)
		loadNode(root, root_node, node_index);

	loadAnimations(root);
	loadSkins(root);

	for (const auto& [hash, handler_ids] : m_resources.vertexHandlers)
	{
		for (const auto& id : handler_ids)
		{
			auto& handler = m_pRenderInterface->getVertexHandler(id);
			handler->allocate();
		}
	}

	for (auto& system : m_systems)
		system->create();
}

void CScene::loadNode(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index, f32 global_scale)
{
	auto& render_packet = m_pRenderInterface->getRenderPacket();

	const auto& node = root.nodes[node_index];

	auto new_node = m_pSceneGraph->createNode(node.name);

	log_info("[Scene] Loadind node: {}:{}.", node.name, node_index);

	m_temporary.nodeToEntity[node_index] = new_node;

	// Parse node local transform
	auto& transform = m_pSceneGraph->getComponent<FTransformComponent>(new_node);
	transform.lposition = glm::make_vec3(node.translation.data());
	transform.lrotation = glm::make_quat(node.rotation.data());
	transform.lscale = glm::make_vec3(node.scale.data());
	transform.lmodel = glm::make_mat4(node.matrix.data());

	// Parse node children
	if (!node.children.empty())
	{
		for (const auto& child_node_index : node.children)
			loadNode(root, m_temporary.nodeToEntity[node_index], child_node_index);
	}

	if(node.mesh >= 0)
		loadMeshComponent(root, m_temporary.nodeToEntity[node_index], node_index);

	if (node.camera >= 0)
		loadCameraComponent(root, m_temporary.nodeToEntity[node_index], node_index);

	if(node.hasExtension<easygltf::extensions::KHR_lights_punctual>())
		loadLightComponent(root, m_temporary.nodeToEntity[node_index], node_index);

	m_pSceneGraph->attachChild(parent, new_node);
	render_packet->stats.nodeCount++;
}

void CScene::loadMeshComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index)
{
	const auto& node = root.nodes[node_index];
	const auto& mesh = root.meshes[node.mesh];

	auto& mesh_component = m_pSceneGraph->addComponent<FMeshComponent>(parent, FMeshComponent{});
	mesh_component.skin = node.skin;

	auto& render_packet = m_pRenderInterface->getRenderPacket();

	// Load mesh primitives
	for (u64 i = 0ull; i < mesh.primitives.size(); ++i)
	{
		const auto& primitive = mesh.primitives[i];

		// TODO: add min max
		glm::vec3 pos_min{}, pos_max{};

		// Load vertices
		
		stl::hash<u32> attribute_hasher;
		stl::vector<render::FVertexAttributePointer> attributes{};
		u64 position_index{ 0ull };

		u64 attributes_hash{ 0ull };
		for (const auto& [name, accessor_idx] : primitive.attributes)
		{
			if (name == "TEXCOORD_1")
				continue;

			const easygltf::Accessor& accessor = root.accessors[accessor_idx];
			auto stream = root.getStream(accessor);

			auto& attrib = attributes.emplace_back();
			attrib.type = get_vertex_attribute_type_by_name(name);
			attrib.data = stream.data;
			attrib.stride = stream.stride;
			attrib.count = stream.count;

			if (attrib.type == render::EVertexAttributeType::ePosition)
			{
				pos_min = !accessor.min.empty() ? glm::vec3(accessor.min[0], accessor.min[1], accessor.min[2]) : glm::vec3(0.f);
				pos_max = !accessor.max.empty() ? glm::vec3(accessor.max[0], accessor.max[1], accessor.max[2]) : glm::vec3(0.f);
				position_index = attributes.size() - 1ull;
			}

			attributes_hash ^= attribute_hasher(utl::cast<u32>(attrib.type) + 1u) << 1;
		}

		// Attributes should be sorted by value type
		stl::sort(attributes.begin(), attributes.end(), [](const auto& lhs, const auto& rhs) { return utl::cast<u32>(lhs.type) < utl::cast<u32>(rhs.type); });

		// Load indices
		render::FVertexAttributePointer indices{};
		if (primitive.indices >= 0)
		{
			const easygltf::Accessor& accessor = root.accessors[primitive.indices];
			auto stream = root.getStream(accessor);
			indices.data = stream.data;
			indices.stride = stream.stride;
			indices.count = stream.count;

			attributes_hash ^= attribute_hasher(utl::cast<u32>(indices.stride)) << 1;
		}

		// Get or create vertex buffer
		render::IVertexHandler* vertex_handler{nullptr};
		u64 current_handler_id{ ~0ull };

		if (auto found = m_resources.vertexHandlers.find(attributes_hash); found != m_resources.vertexHandlers.end())
		{
			for (const auto& handler_id : found->second)
			{
				auto& handler = m_pRenderInterface->getVertexHandler(handler_id);

				vertex_handler = handler.get();
				current_handler_id = handler_id;
			}
		}

		if(!vertex_handler)
		{
			render::FVertexHandlerCreateInfo handler_ci{};
			handler_ci.indexSize = indices.stride;
			handler_ci.name = mesh.name;

			for (const auto& attrib : attributes)
			{
				auto& desc = handler_ci.descriptions.emplace_back();
				desc.type = attrib.type;
				desc.size = attrib.stride;
			}

			auto& handlers = m_resources.vertexHandlers[attributes_hash];

			auto handler_name = stl::format("{}_{}_{}", mesh.name, attributes_hash, handlers.size());
			auto handler_id = m_pRenderInterface->createVertexHandler(handler_name, handler_ci);
			handlers.emplace_back(handler_id);

			auto& handler = m_pRenderInterface->getVertexHandler(handler_id);
			vertex_handler = handler.get();
			current_handler_id = handler_id;

			render_packet->stats.perTypeVBOCount[handler->getHash()]++;
			render_packet->stats.vboCount++;
		}

		// Create mesh primitive
		FMeshPrimitive mesh_primitive{};
		mesh_primitive.vboId = current_handler_id;
		mesh_primitive.setDimensions(pos_min, pos_max);
		mesh_primitive.lods[0] = vertex_handler->addMeshData(attributes, indices);

		// NOTE: Updating material create info for future shader creation
		if (primitive.material >= 0)
			mesh_primitive.materialId = primitive.material;

		mesh_component.primitives.emplace_back(mesh_primitive);
		render_packet->stats.primitiveCount++;
	}
}

void CScene::loadCameraComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index)
{
	const auto& node = root.nodes[node_index];
	const auto& camera = root.cameras[node.camera];

	FCameraComponent camera_component;
	if (camera.type == easygltf::base::enumerators::camera_type::orthographic)
	{
		camera_component.type = ECameraType::eOrthographic;
		camera_component.xmag = camera.orthographic.xmag;
		camera_component.ymag = camera.orthographic.ymag;
		camera_component.nearPlane = camera.orthographic.znear;
		camera_component.farPlane = camera.orthographic.zfar;
	}
	else if (camera.type == easygltf::base::enumerators::camera_type::perspective)
	{
		camera_component.type = ECameraType::ePerspective;
		camera_component.fieldOfView = camera.perspective.yfov;
		camera_component.nearPlane = camera.perspective.znear;
		camera_component.farPlane = camera.perspective.zfar;
	}

	m_pSceneGraph->addComponent<FCameraComponent>(parent, stl::move(camera_component));
}

void CScene::loadLightComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index)
{
	const auto& node = root.nodes[node_index];

	auto& light = root.khrLightsPunctual.lights[node.khrLightsPunctual.light];
	if (light.type == easygltf::extensions::KHR_lights_punctual::enumerators::light_type::directional)
	{
		FDirectionalLightComponent light_component;
		light_component.color = glm::make_vec3(light.color.data());
		light_component.intencity = light.intensity;
		m_pSceneGraph->addComponent<FDirectionalLightComponent>(parent, stl::move(light_component));
	}
	else if (light.type == easygltf::extensions::KHR_lights_punctual::enumerators::light_type::spot)
	{
		FSpotLightComponent light_component;
		light_component.color = glm::make_vec3(light.color.data());
		light_component.innerAngle = light.spot.innerConeAngle;
		light_component.outerAngle = light.spot.outerConeAngle;
		light_component.intencity = light.intensity;
		m_pSceneGraph->addComponent<FSpotLightComponent>(parent, stl::move(light_component));
	}
	else if (light.type == easygltf::extensions::KHR_lights_punctual::enumerators::light_type::point)
	{
		FPointLightComponent light_component;
		light_component.color = glm::make_vec3(light.color.data());
		light_component.intencity = light.intensity;
		light_component.radius = (stl::max)(light.range, 1.0);
		m_pSceneGraph->addComponent<FPointLightComponent>(parent, stl::move(light_component));
	}
}

void CScene::loadAnimations(const easygltf::FGLTFRoot& root)
{
	auto scene_root = m_pSceneGraph->getRoot();
	auto& root_component = m_pSceneGraph->getComponent<FRootComponent>(scene_root);

	for (auto& anim : root.animations)
	{
		FMeshAnimation animation;
		animation.name = anim.name;

		for (auto& samp : anim.samplers)
		{
			FAnimationSampler sampler{};

			if (samp.interpolation == "LINEAR")
				sampler.interpolation = EInterpolationType::eLinear;
			if (samp.interpolation == "STEP")
				sampler.interpolation = EInterpolationType::eStep;
			if (samp.interpolation == "CUBICSPLINE")
				sampler.interpolation = EInterpolationType::eCubicSpline;

			// Read sampler input time values
			{
				const easygltf::Accessor& accessor = root.accessors[samp.input];
				auto stream = root.getStream(accessor);

				// TODO: add other type support
				assert(accessor.componentType == easygltf::base::enumerators::component_type::float_type);

				f32* buf = new f32[accessor.count];
				stl::memcpy(buf, stream.data, stream.count * sizeof(f32));
				for (u64 index = 0; index < stream.count; index++)
					sampler.inputs.push_back(buf[index]);

				delete[] buf;

				for (auto input : sampler.inputs)
				{
					if (input < animation.start)
						animation.start = input;
					if (input > animation.end)
						animation.end = input;
				}
			}

			// Read sampler output T/R/S values 
			{
				const easygltf::Accessor& accessor = root.accessors[samp.output];
				auto stream = root.getStream(accessor);

				assert(accessor.componentType == easygltf::base::enumerators::component_type::float_type);

				if(accessor.type == easygltf::base::enumerators::accessor_type::vec3_type)
				{
					glm::vec3* buf = new glm::vec3[stream.count];
					stl::memcpy(buf, stream.data, stream.count * sizeof(glm::vec3));
					for (u64 index = 0; index < stream.count; index++)
						sampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));

					delete[] buf;
				}
				else if (accessor.type == easygltf::base::enumerators::accessor_type::vec4_type)
				{
					glm::vec4* buf = new glm::vec4[stream.count];
					memcpy(buf, stream.data, stream.count * sizeof(glm::vec4));
					for (u64 index = 0; index < stream.count; index++)
						sampler.outputsVec4.push_back(buf[index]);

					delete[] buf;
				}
			}

			animation.samplers.push_back(sampler);
		}

		// Load animation channdls
		for (auto& source : anim.channels)
		{
			FAnimationChannel channel{};

			if (source.target.path == "rotation")
				channel.path = EPathType::eRotation;
			if (source.target.path == "translation")
				channel.path = EPathType::eTranslation;
			if (source.target.path == "scale")
				channel.path = EPathType::eScale;

			if (source.target.path == "weights") {
				log_warning("[Scene] Weights channel is not yet supported. Skipping.");
				continue;
			}
			channel.samplerIndex = source.sampler;

			auto target = m_temporary.nodeToEntity.find(source.target.node);
			if (target != m_temporary.nodeToEntity.end())
				channel.node = target->second;
			else
				continue;

			animation.channels.push_back(channel);
		}

		root_component.animations.emplace_back(animation);
	}

	log_info("[Scene] Loaded {} animations.", root_component.animations.size());
}

void CScene::loadSkins(const easygltf::FGLTFRoot& root)
{
	auto scene_root = m_pSceneGraph->getRoot();
	auto& root_component = m_pSceneGraph->getComponent<FRootComponent>(scene_root);

	for (auto& source : root.skins)
	{
		FMeshSkin skin{};
		skin.name = source.name;

		// Find skeleton root node
		if (source.skeleton > -1)
		{
			auto target = m_temporary.nodeToEntity.find(source.skeleton);
			if (target != m_temporary.nodeToEntity.end())
				skin.root = target->second;
		}

		// Find joint nodes
		for (int jointIndex : source.joints)
		{
			auto target = m_temporary.nodeToEntity.find(jointIndex);
			if (target != m_temporary.nodeToEntity.end())
				skin.joints.emplace_back(target->second);
		}

		// Get inverse bind matrices from buffer
		if (source.inverseBindMatrices > -1)
		{
			const easygltf::Accessor& accessor = root.accessors[source.inverseBindMatrices];
			auto stream = root.getStream(accessor);
			skin.inverseBindMatrices.resize(stream.count);
			stl::memcpy(skin.inverseBindMatrices.data(), stream.data, stream.count * sizeof(glm::mat4));
		}

		root_component.skins.emplace_back(skin);
	}

	log_info("[Scene] Loaded {} skins (skeletons).", root_component.skins.size());
}