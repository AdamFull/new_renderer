#include "AAMFileSystem.h"
#include "AAMInputFile.h"
#include "AAMIterator.h"

using namespace engine::filesystem;

AAMFileSystem::AAMFileSystem(AAssetManager* assetManager) :
		m_pAssetManager(assetManager)
{

}

AAMFileSystem::~AAMFileSystem()
{

}

bool AAMFileSystem::Exists(const std::string& path)
{
	return isDirectory(path) || isFile(path);
}

size_t AAMFileSystem::FileSize(const std::string& path)
{
	AAsset* asset = AAssetManager_open(m_pAssetManager, path.c_str(), AASSET_MODE_UNKNOWN);
	if (asset)
	{
		off64_t fsize = AAsset_getLength64(asset);
		AAsset_close(asset);

		return static_cast<size_t>(fsize);
	}

	return 0ull;
}

bool AAMFileSystem::IsDirectory(const std::string& path)
{
	AAssetDir* assetDir = AAssetManager_openDir(m_pAssetManager, path.c_str());
	if (assetDir) 
	{
		AAssetDir_close(assetDir);
		return true;
	}

	return false;
}

bool AAMFileSystem::IsFile(const std::string& path)
{
	AAsset* asset = AAssetManager_open(m_pAssetManager, path.c_str(), AASSET_MODE_UNKNOWN);
	if (asset) 
	{
		AAsset_close(asset);
		return true;
	}

	return false;
}

std::unique_ptr<IInputFileInterface> AAMFileSystem::OpenFile(const std::string& path)
{
	return std::make_unique<AAMInputFile>(m_pAssetManager, path);
}

std::unique_ptr<IDirectoryIterator> AAMFileSystem::Walk(const std::string& path, bool recursive)
{
	std::unique_ptr<IDirectoryIterator> pIterator;

	/*if(recursive)
		pIterator = std::make_unique<CAAMRecursiveDirectoryIterator>(m_pAssetManager, path);
	else
		pIterator = std::make_unique<AAMDirectoryIterator>(m_pAssetManager, path);*/

	return pIterator;
}

EFilesystemModeFlags AAMFileSystem::GetFilesystemMode() const
{
	return static_cast<uint32_t>(EFilesystemModeFlagBits::eRead);
}