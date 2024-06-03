#include "AAMFileSystem.h"
#include "AAMInputFile.h"
#include "AAMIterator.h"

using namespace engine::filesystem;

CAAMFileSystem::CAAMFileSystem(AAssetManager* assetManager) :
		m_pAssetManager(assetManager)
{

}

CAAMFileSystem::~CAAMFileSystem()
{

}

bool CAAMFileSystem::Exists(const std::string& path)
{
	return isDirectory(path) || isFile(path);
}

size_t CAAMFileSystem::FileSize(const std::string& path)
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

bool CAAMFileSystem::IsDirectory(const std::string& path)
{
	AAssetDir* assetDir = AAssetManager_openDir(m_pAssetManager, path.c_str());
	if (assetDir) 
	{
		AAssetDir_close(assetDir);
		return true;
	}

	return false;
}

bool CAAMFileSystem::IsFile(const std::string& path)
{
	AAsset* asset = AAssetManager_open(m_pAssetManager, path.c_str(), AASSET_MODE_UNKNOWN);
	if (asset) 
	{
		AAsset_close(asset);
		return true;
	}

	return false;
}

std::unique_ptr<IInputFileInterface> CAAMFileSystem::OpenFile(const std::string& path)
{
	return std::make_unique<CAAMInputFile>(m_pAssetManager, path);
}

std::unique_ptr<IDirectoryIterator> CAAMFileSystem::Walk(const std::string& path, bool recursive)
{
	std::unique_ptr<IDirectoryIterator> pIterator;

	/*if(recursive)
		pIterator = std::make_unique<CAAMRecursiveDirectoryIterator>(m_pAssetManager, path);
	else
		pIterator = std::make_unique<CAAMDirectoryIterator>(m_pAssetManager, path);*/

	return pIterator;
}

EFilesystemModeFlags CAAMFileSystem::GetFilesystemMode() const
{
	return static_cast<uint32_t>(EFilesystemModeFlagBits::eRead);
}