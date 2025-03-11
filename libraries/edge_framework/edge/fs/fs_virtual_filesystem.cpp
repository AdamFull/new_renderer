#include "fs_virtual_filesystem.h"

#include "../util/fs_helper.h"

#include "fs_directory_iterator.h"
#include "fs_file.h"
#include "fs_filesystem.h"

namespace edge
{
	namespace fs
	{
		VirtualFilesystem::~VirtualFilesystem()
		{

		}

		auto VirtualFilesystem::Mount(std::string_view mount_point, IFilesystem* filesystem) -> void
		{
			filesystems.emplace(mount_point, filesystem);
		}

		auto VirtualFilesystem::Unmount(std::string_view mount_point) -> void
		{
			auto found = filesystems.find(mount_point);
			if (found != filesystems.end())
				filesystems.erase(found);
		}

		auto VirtualFilesystem::OpenFile(std::string_view path, FileModeFlags mode) -> IFile*
		{
			auto* filesystem = GetFilesystem(path);
			if (!filesystem)
				return nullptr;

			return filesystem->OpenFile(fsh::get_local_path(path), mode);
		}

		auto VirtualFilesystem::FileSize(std::string_view path) -> size_t
		{
			auto* file = OpenFile(path, FileModeFlag::eRead | FileModeFlag::eBin);
			if (!file)
				return 0ull;

			file->Seek(0, SeekOrigin::eEnd);
			auto size = file->Tell();
			delete file;

			return size;
		}

		auto VirtualFilesystem::ReadFile(std::string_view path, std::string& buffer) -> bool
		{
			auto* file = OpenFile(path, FileModeFlag::eRead | FileModeFlag::eBin);
			file->Seek(0, SeekOrigin::eEnd);
			auto size = file->Tell();
			buffer.resize(size);
			auto bytes_readed = file->Read(buffer);
			delete file;

			return bytes_readed == buffer.size();
		}

		auto VirtualFilesystem::ReadFile(std::string_view path, std::vector<uint8_t>& buffer) -> bool
		{
			auto* file = OpenFile(path, FileModeFlag::eRead | FileModeFlag::eBin);
			file->Seek(0, SeekOrigin::eEnd);
			auto size = file->Tell();
			buffer.resize(size);
			auto bytes_readed = file->Read(buffer);
			delete file;

			return bytes_readed == buffer.size();
		}

		auto VirtualFilesystem::WriteFile(std::string_view path, std::string_view buffer) -> bool
		{
			auto* file = OpenFile(path, FileModeFlag::eWrite | FileModeFlag::eBin);
			auto bytes_writen = file->Write(buffer.data(), buffer.size());
			delete file;

			return bytes_writen == buffer.size();
		}

		auto VirtualFilesystem::WriteFile(std::string_view path, std::span<const uint8_t> buffer) -> bool
		{
			auto* file = OpenFile(path, FileModeFlag::eWrite | FileModeFlag::eBin);
			auto bytes_writen = file->Write(buffer.data(), buffer.size());
			delete file;

			return bytes_writen == buffer.size();
		}

		auto VirtualFilesystem::CreateDirectories(std::string_view path) -> bool
		{
			auto* filesystem = GetFilesystem(path);
			if (!filesystem)
				return false;

			return filesystem->CreateDirectories(fsh::get_local_path(path));
		}

		auto VirtualFilesystem::CreateDirectory(std::string_view path) -> bool
		{
			auto* filesystem = GetFilesystem(path);
			if (!filesystem)
				return false;

			return filesystem->CreateDirectory(fsh::get_local_path(path));
		}

		auto VirtualFilesystem::Exists(std::string_view path) -> bool
		{
			auto* filesystem = GetFilesystem(path);
			if (!filesystem)
				return false;

			return filesystem->Exists(fsh::get_local_path(path));
		}

		auto VirtualFilesystem::IsDirectory(std::string_view path) -> bool
		{
			auto* filesystem = GetFilesystem(path);
			if (!filesystem)
				return false;

			return filesystem->IsDirectory(fsh::get_local_path(path));
		}

		auto VirtualFilesystem::IsFile(std::string_view path) -> bool
		{
			auto* filesystem = GetFilesystem(path);
			if (!filesystem)
				return false;

			return filesystem->IsFile(fsh::get_local_path(path));
		}

		auto VirtualFilesystem::Remove(std::string_view path) -> bool
		{
			auto* filesystem = GetFilesystem(path);
			if (!filesystem)
				return false;

			return filesystem->Remove(fsh::get_local_path(path));
		}

		auto VirtualFilesystem::Walk(std::string_view path, bool recursive) -> IDirectoryIterator*
		{
			auto* filesystem = GetFilesystem(path);
			if (!filesystem)
				return nullptr;

			return filesystem->Walk(fsh::get_local_path(path), recursive);
		}
	
		auto VirtualFilesystem::GetFilesystem(std::string_view path) -> IFilesystem*
		{
			auto fs_prefix = fsh::get_mount_point(path);
			auto found = filesystems.find(std::string(fs_prefix));
			if (found != filesystems.end())
				return found->second;

			return nullptr;
		}
	}
}