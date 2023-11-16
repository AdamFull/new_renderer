#include "Scene.h"

#include <world/SceneGraph.h>
#include <world/ecs/components/components.h>

// TODO: create my own scene format, but based on gltf
#include <parser/easygltf.h>

#include <filesystem/vfs_helper.h>
#include <logger/logger.h>
#include <glm/gtc/type_ptr.hpp>

using namespace engine::world;
using namespace engine::world::ecs;
using namespace engine::filesystem;

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

CScene::CScene(IVirtualFileSystemInterface* vfs)
{
	m_pVFS = vfs;

	m_pSceneGraph = new CSceneGraph();
	m_pScene = new easygltf::CGltfProject();
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

	loadTextures();
	loadMaterials();

	auto& gltf = m_pScene->getGltf();

	const easygltf::Scene& scene = gltf.scenes[gltf.scene > -1 ? gltf.scene : 0]; 
	m_root = static_cast<uint32_t>(m_pSceneGraph->createNode(scene.name));

	for (const auto& node_index : scene.nodes)
		loadNode(m_root, node_index);
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

void CScene::loadNode(const uint32_t parent, uint32_t node_index, float global_scale)
{
	const auto& gltf = m_pScene->getGltf();
	const auto& node = gltf.nodes[node_index];

	auto new_node = m_pSceneGraph->createNode(node.name);

	m_nodeToEntityMap[node_index] = static_cast<uint32_t>(new_node);

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
}

void CScene::loadMeshComponent(const uint32_t parent, uint32_t node_index)
{
	const auto& gltf = m_pScene->getGltf();
	const auto& node = gltf.nodes[node_index];
	const auto& mesh = gltf.meshes[node.mesh];

	auto entity = static_cast<entt::entity>(parent);

	std::vector<FMeshPrimitive>* primitives{ nullptr };
	if (node.skin >= 0)
	{
		auto& mesh_component = m_pSceneGraph->addComponent<FDynamicMeshComponent>(entity, FDynamicMeshComponent{});
		mesh_component.skin = node.skin;
		mesh_component.head = 0; // TODO
		primitives = &mesh_component.primitives;
	}
	else
	{
		auto& mesh_component = m_pSceneGraph->addComponent<FStaticMeshComponent>(entity, FStaticMeshComponent{});
		primitives = &mesh_component.primitives;
	}

	// Load mesh primitives
	for (size_t i = 0ull; i < mesh.primitives.size(); ++i)
	{
		std::vector<uint8_t> index_buffer;
		std::vector<uint8_t> vertex_buffer;

		const auto& primitive = mesh.primitives[i];

		uint32_t index_start = m_lastIndex;
		uint32_t vertex_start = m_lastVertex;
		uint32_t index_count{ 0u }, vertex_count{ 0u };
		glm::vec3 pos_min{}, pos_max{};

		bool has_skin{ false }, has_normals{ false }, has_tangents{ false };

		// Load vertices
		{
			const float* position_buffer{ nullptr };
			const float* normals_buffer{ nullptr };
			const float* texcoord_buffer{ nullptr };
			const float* colors_buffer{ nullptr };
			const float* tangents_buffer{ nullptr };
			uint32_t num_color_components{ 0u };
			const uint16_t* joints_buffer{ nullptr };
			const float* weights_buffer{ nullptr };

			// Position attribute is required
			assert(primitive.attributes.count("POSITION"));
			easygltf::Accessor* position_accessor{ nullptr };
			position_buffer = m_pScene->getBufferPointer<const float>(primitive.attributes.find("POSITION")->second, position_accessor);
			pos_min = glm::vec3(position_accessor->min[0], position_accessor->min[1], position_accessor->min[2]);
			pos_max = glm::vec3(position_accessor->max[0], position_accessor->max[1], position_accessor->max[2]);

			// Load model normals
			if (auto found = primitive.attributes.find("NORMAL"); found != primitive.attributes.end())
			{
				easygltf::Accessor* normals_accessor{ nullptr };
				normals_buffer = m_pScene->getBufferPointer<const float>(found->second, normals_accessor);
				has_normals = true;
			}

			// Load UV coordinates
			if (auto found = primitive.attributes.find("TEXCOORD_0"); found != primitive.attributes.end())
			{
				easygltf::Accessor* texcoord_accessor{ nullptr };
				texcoord_buffer = m_pScene->getBufferPointer<const float>(found->second, texcoord_accessor);
			}

			// Load mesh color data
			if (auto found = primitive.attributes.find("COLOR_0"); found != primitive.attributes.end())
			{
				easygltf::Accessor* color_accessor{ nullptr };
				colors_buffer = m_pScene->getBufferPointer<const float>(found->second, color_accessor);
				num_color_components = color_accessor->componentType == easygltf::base::enumerators::component_type::float_vec3_type ? 3 : 4;
			}

			// Load tangent
			if (auto found = primitive.attributes.find("TANGENT"); found != primitive.attributes.end())
			{
				easygltf::Accessor* tangent_accessor{ nullptr };
				tangents_buffer = m_pScene->getBufferPointer<const float>(found->second, tangent_accessor);
				has_tangents = true;
			}

			// Load joints
			if (auto found = primitive.attributes.find("JOINTS_0"); found != primitive.attributes.end())
			{
				easygltf::Accessor* joints_accessor{ nullptr };
				joints_buffer = m_pScene->getBufferPointer<const uint16_t>(found->second, joints_accessor);
			}

			// Load weight
			if (auto found = primitive.attributes.find("WEIGHTS_0"); found != primitive.attributes.end())
			{
				easygltf::Accessor* weigths_accessor{ nullptr };
				weights_buffer = m_pScene->getBufferPointer<const float>(found->second, weigths_accessor);
			}

			has_skin = (joints_buffer && weights_buffer);

			vertex_count = static_cast<uint32_t>(position_accessor->count);

			// TODO: push vertex data here
		}

		// Load indices
		size_t index_size{ 1ull };
		if (primitive.indices >= 0)
		{
			easygltf::Accessor* accessor{ nullptr };
			const char* buffer = m_pScene->getBufferPointer<const char>(primitive.indices, accessor);
			

			switch (accessor->componentType)
			{
			case easygltf::base::enumerators::component_type::uint_type: { index_size = 4ull; } break;
			case easygltf::base::enumerators::component_type::ushort_type: { index_size = 2ull; } break;
			case easygltf::base::enumerators::component_type::ubyte_type: { index_size = 1ull; } break;
			default:
			{
				std::stringstream ss;
				ss << "Index component type " << accessor->componentType << " not supported!";
				log_warning("Warning while loading gltf scene: {}", ss.str());
				return;
			} break;
			}

			std::vector<char> temp(accessor->count * index_size);
			std::memcpy(temp.data(), buffer, accessor->count * index_size);
			index_buffer.insert(index_buffer.end(), temp.begin(), temp.end());
		}

		FMeshPrimitive mesh_primitive{};
		mesh_primitive.setDimensions(pos_min, pos_max);

		// TODO: set primitive material here
		//if (!m_materials.empty())
		//{
		//	Here i can handle material create infos, but in future i should set material ids
		//}

		// Convert index to local
		for (size_t shift = 0ull; shift < index_buffer.size() / index_size; shift += index_size)
		{
			switch (index_size)
			{
			case 4: {
				uint32_t index = *reinterpret_cast<uint32_t*>(&index_buffer[i]);
				index -= vertex_start;
				*reinterpret_cast<uint32_t*>(&index_buffer[i]) = index;
			} break;
			case 2:
			{
				uint16_t index = *reinterpret_cast<uint16_t*>(&index_buffer[i]);
				index -= vertex_start;
				*reinterpret_cast<uint16_t*>(&index_buffer[i]) = static_cast<uint16_t>(index);
			} break;
			case 1:
			{
				uint8_t index = index_buffer[i];
				index -= vertex_start;
				index_buffer[i] = index;
			} break;
			default:
				throw std::runtime_error("Unsupported vertex size");
			}
		}

		meshopt_optimizeVertexFetch(vertices, indexBuffer.data(), indexCount, vertices, vertexBuffer.size(), sizeof(FVertex));
		meshopt_optimizeVertexCache(indexBuffer.data(), indexBuffer.data(), indexBuffer.size(), vertexBuffer.size());
	}
}