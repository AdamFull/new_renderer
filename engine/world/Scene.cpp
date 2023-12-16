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

template<class _Ty>
bool check_overflow(_Ty a, _Ty b)
{
	if constexpr (std::is_unsigned<_Ty>::value)
		return a > (std::numeric_limits<_Ty>::max)() - b;
	
	return (b > static_cast<_Ty>(0) && a > (std::numeric_limits<_Ty>::max)() - b) || (b < static_cast<_Ty>(0) && a < (std::numeric_limits<_Ty>::min)() - b);
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

	m_systems.emplace_back(std::make_unique<ecs::CInputSystem>(m_pSceneGraph));
	m_systems.emplace_back(std::make_unique<ecs::CAnimationSystem>(m_pSceneGraph));
	m_systems.emplace_back(std::make_unique<ecs::CScenegraphSystem>(m_pSceneGraph));
	m_systems.emplace_back(std::make_unique<ecs::CSkinningSystem>(m_pSceneGraph, render_interface));

	m_systems.emplace_back(std::make_unique<ecs::CCameraSystem>(m_pSceneGraph, render_interface));
	m_systems.emplace_back(std::make_unique<ecs::CShadowsSystem>(m_pSceneGraph, render_interface));
	m_systems.emplace_back(std::make_unique<ecs::CLightingSystem>(m_pSceneGraph, render_interface));
	m_systems.emplace_back(std::make_unique<ecs::CMeshSystem>(m_pSceneGraph, render_interface));
}

CScene::~CScene()
{
	if (m_pSceneGraph)
	{
		delete m_pSceneGraph;
		m_pSceneGraph = nullptr;
	}
}

void CScene::load(const std::string& path)
{
	auto& render_packet = m_pRenderInterface->getRenderPacket();

	easygltf::CGLTFLoader loader{};
	loader.setFilesystemWrapper(std::make_unique<FSWrapper>(m_pVFS));
	
	easygltf::FGLTFRoot root{};
	if (!loader.load(path, root))
		return;

	// Load textures
	for (const auto& texture : root.textures)
	{
		int32_t texture_source{ texture.source };
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
	auto& materials = render_packet->materials;
	for (size_t idx = 0ull; idx < root.materials.size(); ++idx)
	{
		auto& material = root.materials[idx];
		render::FMaterial material_data{};

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
			material_data.normalScale = static_cast<float>(texture.scale);
			material_data.normalTexture = m_resources.textures.at(texture.index);
		}

		// Attach baked ambient occlusion texture
		if (auto& texture = material.occlusionTexture; texture.index >= 0)
		{
			material_data.occlusionStrenth = static_cast<float>(texture.strength);
			material_data.occlusionTexture = m_resources.textures.at(texture.index);
		}

		// Attach emissive texture
		if (auto& texture = material.emissiveTexture; texture.index >= 0)
			material_data.emissiveTexture = m_resources.textures.at(texture.index);

		material_data.emissiveFactor = glm::make_vec3(material.emissiveFactor.data());
		material_data.roughnessFactor = static_cast<float>(material.pbrMetallicRoughness.roughnessFactor);
		material_data.metallicFactor = static_cast<float>(material.pbrMetallicRoughness.metallicFactor);
		material_data.baseColorFactor = glm::make_vec4(material.pbrMetallicRoughness.baseColorFactor.data());

		if (material.alphaMode == easygltf::base::enumerators::alpha_mode::blend)
			material_data.alphaMode = render::MATERIAL_APLHA_MODE_BLEND;
		else if (material.alphaMode == easygltf::base::enumerators::alpha_mode::mask)
			material_data.alphaMode = render::MATERIAL_APLHA_MODE_MASK;
		else
			material_data.alphaMode = render::MATERIAL_APLHA_MODE_OPAQUE;

		material_data.alphaCutoff = static_cast<float>(material.alphaCutoff);
		material_data.doubleSided = material.doubleSided;

		materials->insert(&material_data, idx);

		render_packet->stats.materialCount++;
	}

	// Loading nodes
	const easygltf::Scene& scene = root.scenes[root.scene > -1 ? root.scene : 0];
	auto root_node = m_pSceneGraph->createRoot(scene.name);

	for (const auto& node_index : scene.nodes)
		loadNode(root, root_node, node_index);



	//auto new_node = m_pSceneGraph->createNode("Terrain");
	//m_pSceneGraph->attachChild(root_node, new_node);
	//
	//auto& mesh_component = m_pSceneGraph->addComponent<FMeshComponent>(new_node, FMeshComponent{});
	//auto& primitive = mesh_component.primitives.emplace_back();
	//primitive.materialId = 0;
	//
	//std::vector<FTerrainVertexAttribute> terrain_attributes{};
	//FTerrainVertexAttribute terrain_indices{};
	//
	//CTerrainGenerator gen(542, 64, 10.f);
	//gen.generate(terrain_attributes, terrain_indices);
	//
	//std::vector<render::FVertexAttributePointer> attributes(terrain_attributes.size());
	//render::FVertexAttributePointer indices{};
	//
	//for (size_t idx = 0ull; idx < terrain_attributes.size(); ++idx)
	//{
	//	const auto& tattr = terrain_attributes.at(idx);
	//	auto& attr = attributes.at(idx);
	//
	//	attr.type = tattr.type;
	//	attr.stride = tattr.stride;
	//	attr.count = tattr.count;
	//	attr.data = &tattr.data[0];
	//}
	//
	//indices.stride = terrain_indices.stride;
	//indices.count = terrain_indices.count;
	//indices.data = &terrain_indices.data[0];
	//
	//render::FVertexHandlerCreateInfo handler_ci{};
	//handler_ci.indexSize = indices.stride;
	//handler_ci.name = "Terrain";
	//
	//for (const auto& attrib : attributes)
	//{
	//	auto& desc = handler_ci.descriptions.emplace_back();
	//	desc.type = attrib.type;
	//	desc.size = attrib.stride;
	//}
	//
	//auto& handlers = m_resources.vertexHandlers[0];
	//
	//auto handler_name = std::format("{}_{}_{}", "Terrain", 0, handlers.size());
	//auto handler_id = m_pRenderInterface->createVertexHandler(handler_name, handler_ci);
	//handlers.emplace_back(handler_id);
	//
	//primitive.vboId = handler_id;
	//
	//auto& handler = m_pRenderInterface->getVertexHandler(handler_id);
	//primitive.lods[0].beginVertex = handler->getLastVertex();
	//primitive.lods[0].vertexCount = handler->addVertices(attributes);
	//primitive.lods[0].beginIndex = handler->getLastIndex();
	//primitive.lods[0].indexCount = handler->addIndices(indices, primitive.lods[0].beginVertex);
	//
	//m_resources.primitiveCount++;

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

void CScene::update(float dt)
{
	TRACY_PROFILE_FUNCTION();

	TRACY_PROFILE_SCOPE("Scene update")

	for (auto& system : m_systems)
		system->update(dt);
}

void CScene::loadNode(const easygltf::FGLTFRoot root, const entt::entity parent, uint32_t node_index, float global_scale)
{
	auto& render_packet = m_pRenderInterface->getRenderPacket();

	const auto& node = root.nodes[node_index];

	auto new_node = m_pSceneGraph->createNode(node.name);

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

	if(node.camera >= 0)
		loadCameraComponent(root, m_temporary.nodeToEntity[node_index], node_index);

	if(node.hasExtension<easygltf::extensions::KHR_lights_punctual>())
		loadLightComponent(root, m_temporary.nodeToEntity[node_index], node_index);

	m_pSceneGraph->attachChild(parent, new_node);
	render_packet->stats.nodeCount++;
}

void CScene::loadMeshComponent(const easygltf::FGLTFRoot root, const entt::entity parent, uint32_t node_index)
{
	const auto& node = root.nodes[node_index];
	const auto& mesh = root.meshes[node.mesh];

	auto& mesh_component = m_pSceneGraph->addComponent<FMeshComponent>(parent, FMeshComponent{});
	mesh_component.skin = node.skin;

	auto& render_packet = m_pRenderInterface->getRenderPacket();

	// Load mesh primitives
	for (size_t i = 0ull; i < mesh.primitives.size(); ++i)
	{
		const auto& primitive = mesh.primitives[i];

		// TODO: add min max
		glm::vec3 pos_min{}, pos_max{};

		// Load vertices
		
		std::hash<uint32_t> attribute_hasher;
		std::vector<render::FVertexAttributePointer> attributes{};

		size_t attributes_hash{ 0ull };
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
			}

			attributes_hash ^= attribute_hasher(static_cast<uint32_t>(attrib.type) + 1u) << 1;
		}

		// Attributes should be sorted by value type
		std::sort(attributes.begin(), attributes.end(), [](const auto& lhs, const auto& rhs) { return static_cast<uint32_t>(lhs.type) < static_cast<uint32_t>(rhs.type); });

		// Load indices
		std::vector<uint32_t> indices_tmp{};
		render::FVertexAttributePointer indices{};
		if (primitive.indices >= 0)
		{
			const easygltf::Accessor& accessor = root.accessors[primitive.indices];
			auto stream = root.getStream(accessor);

			bool force_32_bit_indices{ false };
			if (force_32_bit_indices && stream.stride != 4ull)
			{
				if (stream.stride == 2ull)
				{
					std::vector<uint16_t> temp(stream.count);
					std::memcpy(temp.data(), stream.data, stream.stride * stream.count);
					indices_tmp.insert(indices_tmp.end(), temp.begin(), temp.end());
				}
				else if (stream.stride == 1ull)
					indices_tmp.insert(indices_tmp.end(), stream.data, stream.data + stream.count);

				indices.data = reinterpret_cast<const uint8_t*>(indices_tmp.data());
				indices.stride = sizeof(uint32_t);
			}
			else
			{
				indices.data = stream.data;
				indices.stride = stream.stride;
			}

			indices.count = stream.count;

			attributes_hash ^= attribute_hasher(static_cast<uint32_t>(indices.stride)) << 1;
		}

		// Get or create vertex buffer
		render::IVertexHandler* vertex_handler{nullptr};
		size_t current_handler_id{ ~0ull };

		if (auto found = m_resources.vertexHandlers.find(attributes_hash); found != m_resources.vertexHandlers.end())
		{
			for (const auto& handler_id : found->second)
			{
				auto& handler = m_pRenderInterface->getVertexHandler(handler_id);

				auto cur_vertex_count = handler->getLastVertex();
				auto new_vertex_count = attributes.front().count;

				bool is_overflow{ false };
				switch (indices.stride)
				{
				case 4ull: is_overflow = check_overflow(static_cast<uint32_t>(cur_vertex_count), static_cast<uint32_t>(new_vertex_count)); break;
				case 2ull: is_overflow = check_overflow(static_cast<uint16_t>(cur_vertex_count), static_cast<uint16_t>(new_vertex_count)); break;
				case 1ull: is_overflow = check_overflow(static_cast<uint8_t>(cur_vertex_count), static_cast<uint8_t>(new_vertex_count)); break;
				}

				if (!is_overflow)
				{
					vertex_handler = handler.get();
					current_handler_id = handler_id;
				}
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

			auto handler_name = std::format("{}_{}_{}", mesh.name, attributes_hash, handlers.size());
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

		mesh_primitive.lods[0].beginVertex = vertex_handler->getLastVertex();
		mesh_primitive.lods[0].vertexCount = vertex_handler->addVertices(attributes);
		mesh_primitive.lods[0].beginIndex = vertex_handler->getLastIndex();
		mesh_primitive.lods[0].indexCount = vertex_handler->addIndices(indices, mesh_primitive.lods[0].beginVertex);

		// NOTE: Updating material create info for future shader creation
		if (primitive.material >= 0)
			mesh_primitive.materialId = primitive.material;

		mesh_component.primitives.emplace_back(mesh_primitive);
		render_packet->stats.primitiveCount++;
	}
}

void CScene::loadCameraComponent(const easygltf::FGLTFRoot root, const entt::entity parent, uint32_t node_index)
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

	m_pSceneGraph->addComponent<FCameraComponent>(parent, std::move(camera_component));
}

void CScene::loadLightComponent(const easygltf::FGLTFRoot root, const entt::entity parent, uint32_t node_index)
{
	const auto& node = root.nodes[node_index];

	auto& light = root.khrLightsPunctual.lights[node.khrLightsPunctual.light];
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
		light_component.radius = (std::max)(light.range, 1.0);
		m_pSceneGraph->addComponent<FPointLightComponent>(parent, std::move(light_component));
	}
}

void CScene::loadAnimations(const easygltf::FGLTFRoot root)
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

				float* buf = new float[accessor.count];
				std::memcpy(buf, stream.data, stream.count * sizeof(float));
				for (size_t index = 0; index < stream.count; index++)
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
					std::memcpy(buf, stream.data, stream.count * sizeof(glm::vec3));
					for (size_t index = 0; index < stream.count; index++)
						sampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));

					delete[] buf;
				}
				else if (accessor.type == easygltf::base::enumerators::accessor_type::vec4_type)
				{
					glm::vec4* buf = new glm::vec4[stream.count];
					memcpy(buf, stream.data, stream.count * sizeof(glm::vec4));
					for (size_t index = 0; index < stream.count; index++)
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
				log_warning("Weights channel is not yet supported. Skipping.");
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
}

void CScene::loadSkins(const easygltf::FGLTFRoot root)
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
			std::memcpy(skin.inverseBindMatrices.data(), stream.data, stream.count * sizeof(glm::mat4));
		}

		root_component.skins.emplace_back(skin);
	}
}