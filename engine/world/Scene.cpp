#include "Scene.h"

#include <world/SceneGraph.h>
#include <world/ecs/components/components.h>
#include <world/ecs/systems/systems.h>

// TODO: create my own scene format, but based on gltf
#include <parser/easygltf.h>

#include <meshoptimizer.h>

#include <filesystem/vfs_helper.h>
#include <logger/logger.h>
#include <glm/gtc/type_ptr.hpp>

using namespace engine::world;
using namespace engine::world::ecs;
using namespace engine::filesystem;

size_t get_size_by_type(int32_t component_type, const std::string& type)
{
	size_t size{ 0ull };

	switch (component_type)
	{
	case easygltf::base::enumerators::component_type::byte_type:
	case easygltf::base::enumerators::component_type::ubyte_type: size = 1ull; break;
	case easygltf::base::enumerators::component_type::short_type:
	case easygltf::base::enumerators::component_type::ushort_type: size = 2ull; break;
	case easygltf::base::enumerators::component_type::uint_type:
	case easygltf::base::enumerators::component_type::float_type: size = 4ull; break;
	}

	if (type == easygltf::base::enumerators::accessor_type::vec2_type)
		size *= 2ull;
	else if (type == easygltf::base::enumerators::accessor_type::vec3_type)
		size *= 3ull;
	else if (type == easygltf::base::enumerators::accessor_type::vec4_type)
		size *= 4ull;
	else if (type == easygltf::base::enumerators::accessor_type::mat2_type)
		size *= 4ull;
	else if (type == easygltf::base::enumerators::accessor_type::mat3_type)
		size *= 9ull;
	else if (type == easygltf::base::enumerators::accessor_type::mat4_type)
		size *= 16ull;

	return size;
}

engine::render::EVertexAttributeType get_vertex_attribute_type_by_name(const std::string& name)
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

	bool exists(const std::string& path) override
	{
		return m_pVFS->exists(path);
	}

	std::string expandFilePath(const std::string& path) override
	{
		return fs::normalize(fs::to_posix_path(path));
	}

	bool readFile(const std::string& path, std::vector<uint8_t>& data) override
	{
		return m_pVFS->readFile(path, data);
	}

	bool writeFile(const std::string& path, const std::vector<uint8_t>& data) override
	{
		return m_pVFS->writeFile(path, data);
	}

	size_t fileSize(const std::string& path) override
	{
		return m_pVFS->fileSize(path);
	}
private:
	IVirtualFileSystemInterface* m_pVFS{ nullptr };
};

void convert_indices_space(std::vector<uint8_t>& indices, size_t index_size, int32_t vertex_start)
{
	for (size_t shift = 0ull; shift < indices.size() / index_size; shift += index_size)
	{
		switch (index_size)
		{
		case 4: {
			uint32_t index = *reinterpret_cast<uint32_t*>(&indices[shift]);
			index += vertex_start;
			*reinterpret_cast<uint32_t*>(&indices[shift]) = index;
		} break;
		case 2:
		{
			uint16_t index = *reinterpret_cast<uint16_t*>(&indices[shift]);
			index += vertex_start;
			*reinterpret_cast<uint16_t*>(&indices[shift]) = static_cast<uint16_t>(index);
		} break;
		case 1:
		{
			uint8_t index = indices[shift];
			index += vertex_start;
			indices[shift] = index;
		} break;
		default:
			throw std::runtime_error("Unsupported vertex size");
		}
	}
}

CScene::CScene(IVirtualFileSystemInterface* vfs, render::IRenderInterface* render_interface)
{
	m_pVFS = vfs;
	m_pRenderInterface = render_interface;

	m_pSceneGraph = new CSceneGraph();
	m_pScene = new easygltf::CGltfProject();

	m_systems.emplace_back(std::make_unique<ecs::CAnimationSystem>(m_pSceneGraph));
	m_systems.emplace_back(std::make_unique<ecs::CScenegraphSystem>(m_pSceneGraph));
	m_systems.emplace_back(std::make_unique<ecs::CSkinningSystem>(m_pSceneGraph));
}

CScene::~CScene()
{
	if (m_pSceneGraph)
	{
		delete m_pSceneGraph;
		m_pSceneGraph = nullptr;
	}

	if (m_pScene)
	{
		delete m_pScene;
		m_pScene = nullptr;
	}
}

void CScene::load(const std::string& path)
{
	m_pScene->setFilesystemWrapper(std::make_unique<FSWrapper>(m_pVFS));
	
	if (!m_pScene->load(path))
	{
		log_warning(m_pScene->getWarnings());
		log_error(m_pScene->getErrors());
		return;
	}

	//loadTextures();
	//loadMaterials();

	auto& gltf = m_pScene->getGltf();

	const easygltf::Scene& scene = gltf.scenes[gltf.scene > -1 ? gltf.scene : 0]; 
	auto root = m_pSceneGraph->createRoot(scene.name);

	for (const auto& node_index : scene.nodes)
		loadNode(root, node_index);

	void loadAnimations();
	void loadSkins();
}

void CScene::update(float dt)
{
	for (auto& system : m_systems)
		system->update(dt);
}

void CScene::loadTextures()
{
	auto& gltf = m_pScene->getGltf();

	for (const auto& texture : gltf.textures)
	{
		auto& image = gltf.images.at(texture.source);
		auto& sampler = gltf.images.at(texture.sampler);

		// TODO: add ability to override format and sampler params
		auto image_path = m_pScene->getResourcePath(image.uri);
		m_loadedTextures.emplace_back(m_pRenderInterface->createTexture(image_path));
	}
}

void CScene::loadMaterials()
{

}

void CScene::loadNode(const entt::entity parent, uint32_t node_index, float global_scale)
{
	const auto& gltf = m_pScene->getGltf();
	const auto& node = gltf.nodes[node_index];

	auto new_node = m_pSceneGraph->createNode(node.name);

	m_nodeToEntityMap[node_index] = new_node;

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
			loadNode(m_nodeToEntityMap[node_index], child_node_index);
	}

	if(node.mesh >= 0)
		loadMeshComponent(m_nodeToEntityMap[node_index], node_index);

	if(node.camera >= 0)
		loadCameraComponent(m_nodeToEntityMap[node_index], node_index);

	if(node.hasExtension<easygltf::extensions::KHR_lights_punctual>())
		loadLightComponent(m_nodeToEntityMap[node_index], node_index);

	m_pSceneGraph->attachChild(parent, new_node);
}

void CScene::loadMeshComponent(const entt::entity parent, uint32_t node_index)
{
	const auto& gltf = m_pScene->getGltf();
	const auto& node = gltf.nodes[node_index];
	const auto& mesh = gltf.meshes[node.mesh];

	std::vector<FMeshPrimitive>* primitives{ nullptr };
	if (node.skin >= 0)
	{
		auto& mesh_component = m_pSceneGraph->addComponent<FDynamicMeshComponent>(parent, FDynamicMeshComponent{});
		mesh_component.skin = node.skin;
		primitives = &mesh_component.primitives;
	}
	else
	{
		auto& mesh_component = m_pSceneGraph->addComponent<FStaticMeshComponent>(parent, FStaticMeshComponent{});
		primitives = &mesh_component.primitives;
	}

	// Load mesh primitives
	for (size_t i = 0ull; i < mesh.primitives.size(); ++i)
	{
		const auto& primitive = mesh.primitives[i];

		uint32_t index_start = m_pVertexHandler->getLastIndex();
		uint32_t vertex_start = m_pVertexHandler->getLastVertex();
		uint32_t index_count{ 0u }, vertex_count{ 0u };

		// TODO: add min max
		glm::vec3 pos_min{}, pos_max{};

		// Load vertices
		{
			std::vector<render::FVertexAttributePointer> attributes{};

			for (const auto& [name, accessor_idx] : primitive.attributes)
			{
				auto& attrib = attributes.emplace_back();
				attrib.type = get_vertex_attribute_type_by_name(name);
				
				const easygltf::Accessor& accessor = gltf.accessors[accessor_idx];
				attrib.data = m_pScene->getBufferPointer(accessor);
				attrib.stride = get_size_by_type(accessor.componentType, accessor.type);
				attrib.count = accessor.count;
			}

			// Attributes should be sorted by value type
			std::sort(attributes.begin(), attributes.end(), [](const auto& lhs, const auto& rhs) { return static_cast<uint32_t>(lhs.type) < static_cast<uint32_t>(rhs.type); });

			vertex_count = m_pVertexHandler->addVertices(attributes);
		}

		// Load indices
		if (primitive.indices >= 0)
		{
			render::FVertexAttributePointer indices{};

			const easygltf::Accessor& accessor = gltf.accessors[primitive.indices];
			indices.data = m_pScene->getBufferPointer(accessor);
			indices.stride = get_size_by_type(accessor.componentType, accessor.type);
			indices.count = accessor.count;

			index_count = m_pVertexHandler->addIndices(indices);
		}

		FMeshPrimitive mesh_primitive{};
		mesh_primitive.setDimensions(pos_min, pos_max);

		mesh_primitive.lods[0].begin_index = index_start;
		mesh_primitive.lods[0].index_count = index_count;
		mesh_primitive.lods[0].begin_vertex = vertex_start;
		mesh_primitive.lods[0].vertex_count = vertex_count;

		primitives->emplace_back(mesh_primitive);
		m_primitiveCount++;
	}

	// TODO: sort primitives by material
}

void CScene::loadCameraComponent(const entt::entity parent, uint32_t node_index)
{
	const auto& gltf = m_pScene->getGltf();
	const auto& node = gltf.nodes[node_index];
	const auto& camera = gltf.cameras[node.camera];

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

	m_pSceneGraph->addComponent<FCameraComponent>(parent, std::move(camera_component));
}

void CScene::loadLightComponent(const entt::entity parent, uint32_t node_index)
{
	const auto& gltf = m_pScene->getGltf();
	const auto& node = gltf.nodes[node_index];

	auto& light = gltf.khrLightsPunctual.lights[node.khrLightsPunctual.light];
	if (light.type == easygltf::extensions::KHR_lights_punctual::enumerators::light_type::directional)
	{
		FDirectionalLightComponent light_component;
		light_component.color = glm::make_vec3(light.color.data());
		light_component.intencity = light.intensity;
		m_pSceneGraph->addComponent<FDirectionalLightComponent>(parent, std::move(light_component));
	}
	else if (light.type == easygltf::extensions::KHR_lights_punctual::enumerators::light_type::spot)
	{
		FSpotLightComponent light_component;
		light_component.color = glm::make_vec3(light.color.data());
		light_component.innerAngle = light.spot.innerConeAngle;
		light_component.outerAngle = light.spot.outerConeAngle;
		light_component.intencity = light.intensity;
		m_pSceneGraph->addComponent<FSpotLightComponent>(parent, std::move(light_component));
	}
	else if (light.type == easygltf::extensions::KHR_lights_punctual::enumerators::light_type::point)
	{
		FPointLightComponent light_component;
		light_component.color = glm::make_vec3(light.color.data());
		light_component.intencity = light.intensity;
		light_component.radius = light.range;
		m_pSceneGraph->addComponent<FPointLightComponent>(parent, std::move(light_component));
	}
}

void CScene::loadAnimations()
{
	auto root = m_pSceneGraph->getRoot();
	auto& root_component = m_pSceneGraph->getComponent<FRootComponent>(root);

	const auto& gltf = m_pScene->getGltf();
	for (auto& anim : gltf.animations)
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
				const easygltf::Accessor& accessor = gltf.accessors[samp.input];
				const uint8_t* buffer_pointer = m_pScene->getBufferPointer(accessor);

				assert(accessor.componentType == easygltf::base::enumerators::component_type::float_type);

				float* buf = new float[accessor.count];
				std::memcpy(buf, buffer_pointer, accessor.count * sizeof(float));
				for (size_t index = 0; index < accessor.count; index++)
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
				const easygltf::Accessor& accessor = gltf.accessors[samp.output];
				const uint8_t* buffer_pointer = m_pScene->getBufferPointer(accessor);

				assert(accessor.componentType == easygltf::base::enumerators::component_type::float_type);

				if(accessor.type == easygltf::base::enumerators::accessor_type::vec3_type)
				{
					glm::vec3* buf = new glm::vec3[accessor.count];
					std::memcpy(buf, buffer_pointer, accessor.count * sizeof(glm::vec3));
					for (size_t index = 0; index < accessor.count; index++)
						sampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));

					delete[] buf;
				}
				else if (accessor.type == easygltf::base::enumerators::accessor_type::vec4_type)
				{
					glm::vec4* buf = new glm::vec4[accessor.count];
					memcpy(buf, buffer_pointer, accessor.count * sizeof(glm::vec4));
					for (size_t index = 0; index < accessor.count; index++)
						sampler.outputsVec4.push_back(buf[index]);

					delete buf;
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
				log_warning("Weights channel is not yet supported. Skipping.");
				continue;
			}
			channel.samplerIndex = source.sampler;

			auto target = m_nodeToEntityMap.find(source.target.node);
			if (target != m_nodeToEntityMap.end())
				channel.node = target->second;
			else
				continue;

			animation.channels.push_back(channel);
		}

		root_component.animations.emplace_back(animation);
	}
}

void CScene::loadSkins()
{
	auto root = m_pSceneGraph->getRoot();
	auto& root_component = m_pSceneGraph->getComponent<FRootComponent>(root);

	const auto& gltf = m_pScene->getGltf();
	for (auto& source : gltf.skins)
	{
		FMeshSkin skin{};
		skin.name = source.name;

		// Find skeleton root node
		if (source.skeleton > -1)
		{
			auto target = m_nodeToEntityMap.find(source.skeleton);
			if (target != m_nodeToEntityMap.end())
				skin.root = target->second;
		}

		// Find joint nodes
		for (int jointIndex : source.joints)
		{
			auto target = m_nodeToEntityMap.find(jointIndex);
			if (target != m_nodeToEntityMap.end())
				skin.joints.emplace_back(target->second);
		}

		// Get inverse bind matrices from buffer
		if (source.inverseBindMatrices > -1)
		{
			const easygltf::Accessor& accessor = gltf.accessors[source.inverseBindMatrices];
			const uint8_t* buffer_pointer = m_pScene->getBufferPointer(accessor);
			skin.inverseBindMatrices.resize(accessor.count);
			memcpy(skin.inverseBindMatrices.data(), buffer_pointer, accessor.count * sizeof(glm::mat4));
		}

		root_component.skins.emplace_back(skin);
	}
}