#include "VirtualFileSystemManager.h"
#include "VFSInputFile.h"
#include "VFSOutputFile.h"
#include "DirectoryIterator.h"

#include <util/fs_helper.h>

namespace engine
{
	namespace filesystem
	{
		bool is_read_only(const std::unique_ptr<IVFSInterface>& pFS)
		{
			return pFS && !(pFS->GetFilesystemMode() & static_cast<uint32_t>(EFilesystemModeFlagBits::eWrite));
		}


		VirtualFileSystemManager::~VirtualFileSystemManager()
		{
		}

		void VirtualFileSystemManager::Mount(std::string_view mountpoint, std::unique_ptr<IVFSInterface>&& vfsptr)
		{
			m_vMountedFS.emplace(mountpoint, std::move(vfsptr));
		}

		void VirtualFileSystemManager::Unmount(std::string_view mountpoint)
		{
			auto found = m_vMountedFS.find(std::string(mountpoint));
			if (found != m_vMountedFS.end())
				m_vMountedFS.erase(found);
		}

		void VirtualFileSystemManager::Copy(std::string_view from, std::string_view to)
		{
			auto& pFromFilesystem = GetFilesystem(from);
			auto& pToFilesystem = GetFilesystem(to);

			if (is_read_only(pToFilesystem))
				throw std::runtime_error("Failed. You trying to copy file to read only filesystem.");

			const auto fromMountPoint = fsh::get_mount_point(from);
			const auto toMountPoint = fsh::get_mount_point(to);

			if (fromMountPoint == toMountPoint)
			{
				pFromFilesystem->Copy(fsh::get_local_path(from), fsh::get_local_path(to));
				return;
			}

			auto fromPath = fsh::get_local_path(from);
			if (pFromFilesystem->IsDirectory(fromPath))
			{
				auto iterator = std::make_unique<DirectoryIterator>(pFromFilesystem->Walk(fromPath, true), fromMountPoint);

				for (auto& it = iterator->Begin(); it != iterator->End(); ++it)
					ProcessCopyFile(pFromFilesystem, pToFilesystem, fsh::get_local_path(*it));
			}
			else
				ProcessCopyFile(pFromFilesystem, pToFilesystem, fromPath);
		}

		bool VirtualFileSystemManager::CreateDirectories(std::string_view path)
		{
			auto& pFilesystem = GetFilesystem(path);

			if (!pFilesystem)
				return false;

			if (is_read_only(pFilesystem))
				throw std::runtime_error("Failed. You trying to Create directories in read only filesystem.");

			return pFilesystem->CreateDirectories(fsh::get_local_path(path));
		}

		bool VirtualFileSystemManager::CreateDirectory(std::string_view path)
		{
			auto& pFilesystem = GetFilesystem(path);

			if (!pFilesystem)
				return false;

			if (is_read_only(pFilesystem))
				throw std::runtime_error("Failed. You trying to Create directories in read only filesystem.");

			return pFilesystem->CreateDirectory(fsh::get_local_path(path));
		}

		bool VirtualFileSystemManager::Exists(std::string_view path)
		{
			auto& pFilesystem = GetFilesystem(path);

			if (!pFilesystem)
				return false;

			return pFilesystem->Exists(fsh::get_local_path(path));
		}

		size_t VirtualFileSystemManager::FileSize(std::string_view path)
		{
			auto& pFilesystem = GetFilesystem(path);

			if (!pFilesystem)
				return 0ull;

			return pFilesystem->FileSize(fsh::get_local_path(path));
		}

		bool VirtualFileSystemManager::IsDirectory(std::string_view path)
		{
			auto& pFilesystem = GetFilesystem(path);

			if (!pFilesystem)
				return false;

			return pFilesystem->IsDirectory(fsh::get_local_path(path));
		}

		bool VirtualFileSystemManager::IsFile(std::string_view path)
		{
			auto& pFilesystem = GetFilesystem(path);

			if (!pFilesystem)
				return false;

			return pFilesystem->IsFile(fsh::get_local_path(path));
		}

		bool VirtualFileSystemManager::Remove(std::string_view path)
		{
			auto& pFilesystem = GetFilesystem(path);

			if (!pFilesystem)
				return false;

			return pFilesystem->Remove(fsh::get_local_path(path));
		}

		void VirtualFileSystemManager::Rename(std::string_view original, std::string_view newfile)
		{
			if (fsh::get_mount_point(original) != fsh::get_mount_point(newfile))
				return;

			auto& pFilesystem = GetFilesystem(original);

			if (!pFilesystem)
				return;

			pFilesystem->Rename(fsh::get_local_path(original), fsh::get_local_path(newfile));
		}

		std::unique_ptr<IVFSOutputFileInterface> VirtualFileSystemManager::CreateFile(std::string_view path)
		{
			const std::lock_guard<std::mutex> lock(m_SynchronizationMutex);

			auto& pFilesystem = GetFilesystem(path);
			if (!pFilesystem)
				return nullptr;

			if (is_read_only(pFilesystem))
				throw std::runtime_error("Failed. You trying to Create file in read only filesystem.");

			return std::make_unique<VFSOutFile>(pFilesystem->CreateFile(fsh::get_local_path(path)));
		}

		std::unique_ptr<IVFSInputFileInterface> VirtualFileSystemManager::OpenFile(std::string_view path)
		{
			const std::lock_guard<std::mutex> lock(m_SynchronizationMutex);

			auto& pFilesystem = GetFilesystem(path);
			if (!pFilesystem)
				return nullptr;

			auto local_path = fsh::get_local_path(path);

			if (pFilesystem->IsDirectory(local_path))
				return nullptr;

			return std::make_unique<VFSInFile>(pFilesystem->OpenFile(local_path));
		}

		std::unique_ptr<IDirectoryIteratorInterface> VirtualFileSystemManager::Walk(std::string_view path, bool recursive)
		{
			auto& pFilesystem = GetFilesystem(path);
			if (!pFilesystem)
				return nullptr;

			return std::make_unique<DirectoryIterator>(pFilesystem->Walk(fsh::get_local_path(path), recursive), fsh::get_mount_point(path));
		}

		bool VirtualFileSystemManager::ReadFile(std::string_view path, char* data, size_t size)
		{
			auto pFile = OpenFile(path);
			if (pFile && pFile->IsOpen())
			{
				pFile->Read(data, size);
				return true;
			}

			return false;
		}

		bool VirtualFileSystemManager::ReadFile(std::string_view path, std::string& data)
		{
			auto pFile = OpenFile(path);
			if (pFile && pFile->IsOpen())
			{
				pFile->Read(data);
				return true;
			}

			return false;
		}

		bool VirtualFileSystemManager::ReadFile(std::string_view path, utl::vector<uint8_t>& data)
		{
			auto pFile = OpenFile(path);
			if (pFile && pFile->IsOpen())
			{
				pFile->Read(data);
				return true;
			}

			return false;
		}

		bool VirtualFileSystemManager::ReadFile(std::string_view path, utl::vector<uint32_t>& data)
		{
			auto pFile = OpenFile(path);
			if (pFile && pFile->IsOpen())
			{
				pFile->Read(data);
				return true;
			}

			return false;
		}

		bool VirtualFileSystemManager::WriteFile(std::string_view path, const std::string& data)
		{
			auto pFile = CreateFile(path);
			if (pFile && pFile->IsOpen())
			{
				pFile->Write(data);
				return true;
			}

			return false;
		}

		bool VirtualFileSystemManager::WriteFile(std::string_view path, const utl::vector<uint8_t>& binary)
		{
			auto pFile = CreateFile(path);
			if (pFile && pFile->IsOpen())
			{
				pFile->Write(binary);
				return true;
			}

			return false;
		}

		const std::unique_ptr<IVFSInterface>& VirtualFileSystemManager::GetFilesystem(std::string_view path)
		{
			static std::unique_ptr<IVFSInterface> pEmpty{ nullptr };

			auto fs_prefix = fsh::get_mount_point(path);
			auto found = m_vMountedFS.find(std::string(fs_prefix));
			if (found != m_vMountedFS.end())
				return found->second;

			return pEmpty;
		}

		void VirtualFileSystemManager::ProcessCopyFile(const std::unique_ptr<IVFSInterface>& pFSFrom, const std::unique_ptr<IVFSInterface>& pFSTo, std::string_view path)
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
	}
}