#pragma once

#include <string_view>
#include <unordered_map>

#include "fs_file.h"
#include "fs_directory_iterator.h"

namespace edge::fs
{
	class IFilesystem;

	class VirtualFilesystem {
	public:
		~VirtualFilesystem();

		auto mount(std::string_view mount_point, IFilesystem* filesystem) -> void;
		auto unmount(std::string_view mount_point) -> void;

		auto open_file(std::string_view path, FileModeFlags mode) -> IFile*;
		auto file_size(std::string_view path) -> size_t;
		auto read_file(std::string_view path, std::string& buffer) -> bool;
		auto read_file(std::string_view path, std::vector<uint8_t>& buffer) -> bool;
		auto write_file(std::string_view path, std::string_view buffer) -> bool;
		auto write_file(std::string_view path, std::span<const uint8_t> buffer) -> bool;

		auto create_directories(std::string_view path) -> bool;
		auto create_directory(std::string_view path) -> bool;

		auto exists(std::string_view path) -> bool;
		auto is_directory(std::string_view path) -> bool;
		auto is_file(std::string_view path) -> bool;

		auto remove(std::string_view path) -> bool;

		auto walk(std::string_view path, bool recursive = false) -> DirectoryIterator;
	private:
		auto get_filesystem(std::string_view path) -> IFilesystem*;

		std::unordered_map<std::string_view, IFilesystem*> filesystems;
	};
}