#include "fs_virtual_filesystem.h"

#include "fs_helper.h"

#include "fs_directory_iterator.h"
#include "fs_file.h"
#include "fs_filesystem.h"

namespace edge::fs
{
	VirtualFilesystem::~VirtualFilesystem() {
	}

	auto VirtualFilesystem::mount(std::string_view mount_point, IFilesystem* filesystem) -> void {
		filesystems.emplace(mount_point, filesystem);
	}

	auto VirtualFilesystem::unmount(std::string_view mount_point) -> void {
		auto found = filesystems.find(mount_point);
		if (found != filesystems.end())
			filesystems.erase(found);
	}

	auto VirtualFilesystem::open_file(std::string_view path, FileModeFlags mode) -> IFile* {
		auto* filesystem = get_filesystem(path);
		if (!filesystem)
			return nullptr;

		return filesystem->open_file(fs::get_local_path(path), mode);
	}

	auto VirtualFilesystem::file_size(std::string_view path) -> size_t
	{
		auto* filesystem = get_filesystem(path);
		if (!filesystem)
			return 0ull;

		return filesystem->file_size(fs::get_local_path(path));
	}

	auto VirtualFilesystem::read_file(std::string_view path, std::string& buffer) -> bool
	{
		auto* file = open_file(path, FileModeFlag::eRead | FileModeFlag::eBin);
		buffer.resize(file->size());
		auto bytes_readed = file->read(buffer);
		delete file;

		return bytes_readed == buffer.size();
	}

	auto VirtualFilesystem::read_file(std::string_view path, std::vector<uint8_t>& buffer) -> bool
	{
		auto* file = open_file(path, FileModeFlag::eRead | FileModeFlag::eBin);
		buffer.resize(file->size());
		auto bytes_readed = file->read(buffer);
		delete file;

		return bytes_readed == buffer.size();
	}

	auto VirtualFilesystem::write_file(std::string_view path, std::string_view buffer) -> bool
	{
		auto* file = open_file(path, FileModeFlag::eWrite | FileModeFlag::eBin);
		auto bytes_writen = file->write(buffer.data(), buffer.size());
		delete file;

		return bytes_writen == buffer.size();
	}

	auto VirtualFilesystem::write_file(std::string_view path, std::span<const uint8_t> buffer) -> bool
	{
		auto* file = open_file(path, FileModeFlag::eWrite | FileModeFlag::eBin);
		auto bytes_writen = file->write(buffer.data(), buffer.size());
		delete file;

		return bytes_writen == buffer.size();
	}

	auto VirtualFilesystem::create_directories(std::string_view path) -> bool
	{
		auto* filesystem = get_filesystem(path);
		if (!filesystem)
			return false;

		return filesystem->create_directories(fs::get_local_path(path));
	}

	auto VirtualFilesystem::create_directory(std::string_view path) -> bool
	{
		auto* filesystem = get_filesystem(path);
		if (!filesystem)
			return false;

		return filesystem->create_directory(fs::get_local_path(path));
	}

	auto VirtualFilesystem::exists(std::string_view path) -> bool
	{
		auto* filesystem = get_filesystem(path);
		if (!filesystem)
			return false;

		return filesystem->exists(fs::get_local_path(path));
	}

	auto VirtualFilesystem::is_directory(std::string_view path) -> bool
	{
		auto* filesystem = get_filesystem(path);
		if (!filesystem)
			return false;

		return filesystem->is_directory(fs::get_local_path(path));
	}

	auto VirtualFilesystem::is_file(std::string_view path) -> bool
	{
		auto* filesystem = get_filesystem(path);
		if (!filesystem)
			return false;

		return filesystem->is_file(fs::get_local_path(path));
	}

	auto VirtualFilesystem::remove(std::string_view path) -> bool
	{
		auto* filesystem = get_filesystem(path);
		if (!filesystem)
			return false;

		return filesystem->remove(fs::get_local_path(path));
	}

	auto VirtualFilesystem::walk(std::string_view path, bool recursive) -> DirectoryIterator
	{
		auto* filesystem = get_filesystem(path);
		if (!filesystem)
			return DirectoryIterator{ nullptr };

		return DirectoryIterator{ filesystem->walk(fs::get_local_path(path), recursive) };
	}

	auto VirtualFilesystem::get_filesystem(std::string_view path) -> IFilesystem*
	{
		auto fs_prefix = fs::get_mount_point(path);
		auto found = filesystems.find(fs_prefix);
		if (found != filesystems.end())
			return found->second;

		return nullptr;
	}
}