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

bool AAMFileSystem::Exists(std::string_view path)
{
	return isDirectory(path) || isFile(path);
}

size_t AAMFileSystem::FileSize(std::string_view path)
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

bool AAMFileSystem::IsDirectory(std::string_view path)
{
	AAssetDir* assetDir = AAssetManager_openDir(m_pAssetManager, path.c_str());
	if (assetDir) 
	{
		AAssetDir_close(assetDir);
		return true;
	}

	return false;
}

bool AAMFileSystem::IsFile(std::string_view path)
{
	AAsset* asset = AAssetManager_open(m_pAssetManager, path.c_str(), AASSET_MODE_UNKNOWN);
	if (asset) 
	{
		AAsset_close(asset);
		return true;
	}

	return false;
}

std::unique_ptr<IInputFileInterface> AAMFileSystem::OpenFile(std::string_view path)
{
	return std::make_unique<AAMInputFile>(m_pAssetManager, path);
}

std::unique_ptr<IDirectoryIterator> AAMFileSystem::Walk(std::string_view path, bool recursive)
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