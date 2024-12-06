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


VirtualFileSystemManager::~VirtualFileSystemManager()
{
}

void VirtualFileSystemManager::Mount(const char* mountpoint, std::unique_ptr<IVFSInterface>&& vfsptr)
{
	m_vMountedFS.emplace(mountpoint, std::move(vfsptr));
}

void VirtualFileSystemManager::Unmount(const char* mountpoint)
{
	auto found = m_vMountedFS.find(mountpoint);
	if (found != m_vMountedFS.end())
		m_vMountedFS.erase(found);
}

void VirtualFileSystemManager::Copy(const char* from, const char* to)
{
	auto& pFromFilesystem = GetFilesystem(from);
	auto& pToFilesystem = GetFilesystem(to);

	if (is_read_only(pToFilesystem))
		throw std::runtime_error("Failed. You trying to copy file to read only filesystem.");

	const auto& fromMountPoint = fs::get_mount_point(from);
	const auto& toMountPoint = fs::get_mount_point(to);
	
	if (fromMountPoint == toMountPoint)
	{
		pFromFilesystem->Copy(fs::get_local_path(from).c_str(), fs::get_local_path(to).c_str());
		return;
	}
		
	auto fromPath = fs::get_local_path(from);
	if (pFromFilesystem->IsDirectory(fromPath.c_str()))
	{
		auto iterator = std::make_unique<DirectoryIterator>(pFromFilesystem->Walk(fromPath.c_str(), true), fromMountPoint);

		for (auto& it = iterator->Begin(); it != iterator->End() ; ++it)
			ProcessCopyFile(pFromFilesystem, pToFilesystem, fs::get_local_path(*it).c_str());
	}
	else
		ProcessCopyFile(pFromFilesystem, pToFilesystem, fromPath.c_str());
}

bool VirtualFileSystemManager::CreateDirectories(const char* path)
{
	auto& pFilesystem = GetFilesystem(path);

	if (!pFilesystem)
		return false;

	if (is_read_only(pFilesystem))
		throw std::runtime_error("Failed. You trying to Create directories in read only filesystem.");

	return pFilesystem->CreateDirectories(fs::get_local_path(path).c_str());
}

bool VirtualFileSystemManager::CreateDirectory(const char* path)
{
	auto& pFilesystem = GetFilesystem(path);

	if (!pFilesystem)
		return false;

	if (is_read_only(pFilesystem))
		throw std::runtime_error("Failed. You trying to Create directories in read only filesystem.");

	return pFilesystem->CreateDirectory(fs::get_local_path(path).c_str());
}

bool VirtualFileSystemManager::Exists(const char* path)
{
	auto& pFilesystem = GetFilesystem(path);

	if (!pFilesystem)
		return false;

	return pFilesystem->Exists(fs::get_local_path(path).c_str());
}

size_t VirtualFileSystemManager::FileSize(const char* path)
{
	auto& pFilesystem = GetFilesystem(path);

	if (!pFilesystem)
		return 0ull;

	return pFilesystem->FileSize(fs::get_local_path(path).c_str());
}

bool VirtualFileSystemManager::IsDirectory(const char* path)
{
	auto& pFilesystem = GetFilesystem(path);

	if (!pFilesystem)
		return false;

	return pFilesystem->IsDirectory(fs::get_local_path(path).c_str());
}

bool VirtualFileSystemManager::IsFile(const char* path)
{
	auto& pFilesystem = GetFilesystem(path);

	if (!pFilesystem)
		return false;

	return pFilesystem->IsFile(fs::get_local_path(path).c_str());
}

bool VirtualFileSystemManager::Remove(const char* path)
{
	auto& pFilesystem = GetFilesystem(path);

	if (!pFilesystem)
		return false;

	return pFilesystem->Remove(fs::get_local_path(path).c_str());
}

void VirtualFileSystemManager::Rename(const char* original, const char* newfile)
{
	if (fs::get_mount_point(original) != fs::get_mount_point(newfile))
		return;

	auto& pFilesystem = GetFilesystem(original);

	if (!pFilesystem)
		return;

	pFilesystem->Rename(fs::get_local_path(original).c_str(), fs::get_local_path(newfile).c_str());
}

std::unique_ptr<IVFSOutputFileInterface> VirtualFileSystemManager::CreateFile(const char* path)
{
	const std::lock_guard<std::mutex> lock(m_SynchronizationMutex);

	auto& pFilesystem = GetFilesystem(path);
	if (!pFilesystem)
		return nullptr;

	if (is_read_only(pFilesystem))
		throw std::runtime_error("Failed. You trying to Create file in read only filesystem.");

	return std::make_unique<VFSOutFile>(pFilesystem->CreateFile(fs::get_local_path(path).c_str()));
}

std::unique_ptr<IVFSInputFileInterface> VirtualFileSystemManager::OpenFile(const char* path)
{
	const std::lock_guard<std::mutex> lock(m_SynchronizationMutex);

	auto& pFilesystem = GetFilesystem(path);
	if (!pFilesystem)
		return nullptr;

	auto local_path = fs::get_local_path(path);

	if (pFilesystem->IsDirectory(local_path.c_str()))
		return nullptr;

	return std::make_unique<VFSInFile>(pFilesystem->OpenFile(local_path.c_str()));
}

std::unique_ptr<IDirectoryIteratorInterface> VirtualFileSystemManager::Walk(const char* path, bool recursive)
{
	auto& pFilesystem = GetFilesystem(path);
	if (!pFilesystem)
		return nullptr;

	return std::make_unique<DirectoryIterator>(pFilesystem->Walk(fs::get_local_path(path).c_str(), recursive), fs::get_mount_point(path));
}

bool VirtualFileSystemManager::ReadFile(const char* path, char* data, size_t size)
{
	auto pFile = OpenFile(path);
	if (pFile && pFile->IsOpen())
	{
		pFile->Read(data, size);
		return true;
	}

	return false;
}

bool VirtualFileSystemManager::ReadFile(const char* path, std::string& data)
{
	auto pFile = OpenFile(path);
	if (pFile && pFile->IsOpen())
	{
		pFile->Read(data);
		return true;
	}

	return false;
}

bool VirtualFileSystemManager::ReadFile(const char* path, utl::vector<uint8_t>& data)
{
	auto pFile = OpenFile(path);
	if (pFile && pFile->IsOpen())
	{
		pFile->Read(data);
		return true;
	}

	return false;
}

bool VirtualFileSystemManager::ReadFile(const char* path, utl::vector<uint32_t>& data)
{
	auto pFile = OpenFile(path);
	if (pFile && pFile->IsOpen())
	{
		pFile->Read(data);
		return true;
	}

	return false;
}

bool VirtualFileSystemManager::WriteFile(const char* path, const std::string& data)
{
	auto pFile = CreateFile(path);
	if (pFile && pFile->IsOpen())
	{
		pFile->Write(data);
		return true;
	}

	return false;
}

bool VirtualFileSystemManager::WriteFile(const char* path, const utl::vector<uint8_t>& binary)
{
	auto pFile = CreateFile(path);
	if (pFile && pFile->IsOpen())
	{
		pFile->Write(binary);
		return true;
	}

	return false;
}

const std::unique_ptr<IVFSInterface>& VirtualFileSystemManager::GetFilesystem(const char* path)
{
	static std::unique_ptr<IVFSInterface> pEmpty{ nullptr };

	auto fs_prefix = fs::get_mount_point(path);
	auto found = m_vMountedFS.find(fs_prefix);
	if (found != m_vMountedFS.end())
		return found->second;

	return pEmpty;
}

void VirtualFileSystemManager::ProcessCopyFile(const std::unique_ptr<IVFSInterface>& pFSFrom, const std::unique_ptr<IVFSInterface>& pFSTo, const char* path)
{
	if (pFSFrom->IsDirectory(path))
	{
		pFSTo->CreateDirectories(path);
		return;
	}
	

	auto pInFile = std::make_unique<VFSInFile>(pFSFrom->OpenFile(path));
	auto pOutFile = std::make_unique<VFSOutFile>(pFSTo->CreateFile(path));

	std::string buffer(4096, '\0');

	while (!pInFile->Eof())
	{
		pInFile->Read(buffer.data(), buffer.size());
		pOutFile->Write(buffer.data(), pInFile->Gcount());
	}
}