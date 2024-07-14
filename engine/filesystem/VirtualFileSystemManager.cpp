#include "VirtualFileSystemManager.h"
#include "VFSInputFile.h"
#include "VFSOutputFile.h"
#include "DirectoryIterator.h"
#include "vfs_helper.h"

using namespace engine::filesystem;

bool is_read_only(const std::unique_ptr<IVFSInterface>& pFS)
{
	return pFS && !(pFS->GetFilesystemMode() & static_cast<uint32_t>(EFilesystemModeFlagBits::eWrite));
}


CVirtualFileSystemManager::~CVirtualFileSystemManager()
{
}

void CVirtualFileSystemManager::Mount(const std::string& mountpoint, std::unique_ptr<IVFSInterface>&& vfsptr)
{
	m_vMountedFS.emplace(mountpoint, std::move(vfsptr));
}

void CVirtualFileSystemManager::Unmount(const std::string& mountpoint)
{
	auto found = m_vMountedFS.find(mountpoint);
	if (found != m_vMountedFS.end())
		m_vMountedFS.erase(found);
}

void CVirtualFileSystemManager::Copy(const std::string& from, const std::string& to)
{
	auto& pFromFilesystem = GetFilesystem(from);
	auto& pToFilesystem = GetFilesystem(to);

	if (is_read_only(pToFilesystem))
		throw std::runtime_error("Failed. You trying to copy file to read only filesystem.");

	const auto& fromMountPoint = fs::get_mount_point(from);
	const auto& toMountPoint = fs::get_mount_point(to);
	
	if (fromMountPoint == toMountPoint)
	{
		pFromFilesystem->Copy(fs::get_local_path(from), fs::get_local_path(to));
		return;
	}
		
	auto fromPath = fs::get_local_path(from);
	if (pFromFilesystem->IsDirectory(fromPath))
	{
		auto iterator = std::make_unique<CDirectoryIterator>(pFromFilesystem->Walk(fromPath, true), fromMountPoint);

		for (auto& it = iterator->Begin(); it != iterator->End() ; ++it)
			ProcessCopyFile(pFromFilesystem, pToFilesystem, fs::get_local_path(*it));
	}
	else
		ProcessCopyFile(pFromFilesystem, pToFilesystem, fromPath);
}

bool CVirtualFileSystemManager::CreateDirectories(const std::string& path)
{
	auto& pFilesystem = GetFilesystem(path);

	if (!pFilesystem)
		return false;

	if (is_read_only(pFilesystem))
		throw std::runtime_error("Failed. You trying to Create directories in read only filesystem.");

	return pFilesystem->CreateDirectories(fs::get_local_path(path));
}

bool CVirtualFileSystemManager::CreateDirectory(const std::string& path)
{
	auto& pFilesystem = GetFilesystem(path);

	if (!pFilesystem)
		return false;

	if (is_read_only(pFilesystem))
		throw std::runtime_error("Failed. You trying to Create directories in read only filesystem.");

	return pFilesystem->CreateDirectory(fs::get_local_path(path));
}

bool CVirtualFileSystemManager::Exists(const std::string& path)
{
	auto& pFilesystem = GetFilesystem(path);

	if (!pFilesystem)
		return false;

	return pFilesystem->Exists(fs::get_local_path(path));
}

size_t CVirtualFileSystemManager::FileSize(const std::string& path)
{
	auto& pFilesystem = GetFilesystem(path);

	if (!pFilesystem)
		return 0ull;

	return pFilesystem->FileSize(fs::get_local_path(path));
}

bool CVirtualFileSystemManager::IsDirectory(const std::string& path)
{
	auto& pFilesystem = GetFilesystem(path);

	if (!pFilesystem)
		return false;

	return pFilesystem->IsDirectory(fs::get_local_path(path));
}

bool CVirtualFileSystemManager::IsFile(const std::string& path)
{
	auto& pFilesystem = GetFilesystem(path);

	if (!pFilesystem)
		return false;

	return pFilesystem->IsFile(fs::get_local_path(path));
}

bool CVirtualFileSystemManager::Remove(const std::string& path)
{
	auto& pFilesystem = GetFilesystem(path);

	if (!pFilesystem)
		return false;

	return pFilesystem->Remove(fs::get_local_path(path));
}

void CVirtualFileSystemManager::Rename(const std::string& original, const std::string& newfile)
{
	if (fs::get_mount_point(original) != fs::get_mount_point(newfile))
		return;

	auto& pFilesystem = GetFilesystem(original);

	if (!pFilesystem)
		return;

	pFilesystem->Rename(fs::get_local_path(original), fs::get_local_path(newfile));
}

std::unique_ptr<IVFSOutputFileInterface> CVirtualFileSystemManager::CreateFile(const std::string& path)
{
	const std::lock_guard<std::mutex> lock(m_SynchronizationMutex);

	auto& pFilesystem = GetFilesystem(path);
	if (!pFilesystem)
		return nullptr;

	if (is_read_only(pFilesystem))
		throw std::runtime_error("Failed. You trying to Create file in read only filesystem.");

	return std::make_unique<CVFSOutFile>(pFilesystem->CreateFile(fs::get_local_path(path)));
}

std::unique_ptr<IVFSInputFileInterface> CVirtualFileSystemManager::OpenFile(const std::string& path)
{
	const std::lock_guard<std::mutex> lock(m_SynchronizationMutex);

	auto& pFilesystem = GetFilesystem(path);
	if (!pFilesystem)
		return nullptr;

	auto local_path = fs::get_local_path(path);

	if (pFilesystem->IsDirectory(local_path))
		return nullptr;

	return std::make_unique<CVFSInFile>(pFilesystem->OpenFile(local_path));
}

std::unique_ptr<IDirectoryIteratorInterface> CVirtualFileSystemManager::Walk(const std::string& path, bool recursive)
{
	auto& pFilesystem = GetFilesystem(path);
	if (!pFilesystem)
		return nullptr;

	return std::make_unique<CDirectoryIterator>(pFilesystem->Walk(fs::get_local_path(path), recursive), fs::get_mount_point(path));
}

bool CVirtualFileSystemManager::ReadFile(const std::string& path, char* data, size_t size)
{
	auto pFile = OpenFile(path);
	if (pFile && pFile->IsOpen())
	{
		pFile->Read(data, size);
		return true;
	}

	return false;
}

bool CVirtualFileSystemManager::ReadFile(const std::string& path, std::string& data)
{
	auto pFile = OpenFile(path);
	if (pFile && pFile->IsOpen())
	{
		pFile->Read(data);
		return true;
	}

	return false;
}

bool CVirtualFileSystemManager::ReadFile(const std::string& path, utl::vector<uint8_t>& data)
{
	auto pFile = OpenFile(path);
	if (pFile && pFile->IsOpen())
	{
		pFile->Read(data);
		return true;
	}

	return false;
}

bool CVirtualFileSystemManager::ReadFile(const std::string& path, utl::vector<uint32_t>& data)
{
	auto pFile = OpenFile(path);
	if (pFile && pFile->IsOpen())
	{
		pFile->Read(data);
		return true;
	}

	return false;
}

bool CVirtualFileSystemManager::WriteFile(const std::string& path, const std::string& data)
{
	auto pFile = CreateFile(path);
	if (pFile && pFile->IsOpen())
	{
		pFile->Write(data);
		return true;
	}

	return false;
}

bool CVirtualFileSystemManager::WriteFile(const std::string& path, const utl::vector<uint8_t>& binary)
{
	auto pFile = CreateFile(path);
	if (pFile && pFile->IsOpen())
	{
		pFile->Write(binary);
		return true;
	}

	return false;
}

const std::unique_ptr<IVFSInterface>& CVirtualFileSystemManager::GetFilesystem(const std::string& path)
{
	static std::unique_ptr<IVFSInterface> pEmpty{ nullptr };

	auto fs_prefix = fs::get_mount_point(path);
	auto found = m_vMountedFS.find(fs_prefix);
	if (found != m_vMountedFS.end())
		return found->second;

	return pEmpty;
}

void CVirtualFileSystemManager::ProcessCopyFile(const std::unique_ptr<IVFSInterface>& pFSFrom, const std::unique_ptr<IVFSInterface>& pFSTo, const std::string& path)
{
	if (pFSFrom->IsDirectory(path))
	{
		pFSTo->CreateDirectories(path);
		return;
	}
	

	auto pInFile = std::make_unique<CVFSInFile>(pFSFrom->OpenFile(path));
	auto pOutFile = std::make_unique<CVFSOutFile>(pFSTo->CreateFile(path));

	std::string buffer(4096, '\0');

	while (!pInFile->Eof())
	{
		pInFile->Read(buffer.data(), buffer.size());
		pOutFile->Write(buffer.data(), pInFile->Gcount());
	}
}