#include "Scene.h"

// TODO: create my own scene format, but based on gltf
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

#include <filesystem/vfs_helper.h>

using namespace engine::world;
using namespace engine::filesystem;

bool gltfFileSystemFileExists(const std::string& filename, void* userdata)
{
	auto* vfs = static_cast<IVirtualFileSystemInterface*>(userdata);
	return vfs->exists(filename);
}

std::string gltfFileSystemExpandFilePath(const std::string& filename, void* userdata)
{
	return fs::normalize(fs::to_posix_path(filename));
}

bool gltfFileSystemReadFile(std::vector<uint8_t>* u8data, std::string* err, const std::string& filename, void* userdata)
{
	auto* vfs = static_cast<IVirtualFileSystemInterface*>(userdata);
	return vfs->readFile(filename, *u8data);
}

bool gltfFileSystemWriteFile(std::string* err, const std::string& filename, const std::vector<uint8_t>& u8data, void* userdata)
{
	auto* vfs = static_cast<IVirtualFileSystemInterface*>(userdata);
	return vfs->writeFile(filename, u8data);
}

CScene::CScene(IVirtualFileSystemInterface* vfs)
{
	m_pVFS = vfs;

	m_pSceneGraph = std::make_unique<CSceneGraph>();
}

void CScene::create()
{
	
}

void CScene::load(const std::string& path)
{
	tinygltf::Model gltfModel;
	tinygltf::TinyGLTF gltfContext;
}