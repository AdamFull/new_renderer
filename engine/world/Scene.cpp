#include "Scene.h"

#include <world/SceneGraph.h>

// TODO: create my own scene format, but based on gltf
#include <parser/easygltf.h>

#include <filesystem/vfs_helper.h>
#include <logger/logger.h>

using namespace engine::world;
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