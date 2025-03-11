#pragma once

#include <string_view>
#include <unordered_map>

#include "fs_file.h"
#include "fs_directory_iterator.h"

namespace edge
{
	namespace fs
	{
		class IFilesystem;

		class VirtualFilesystem
		{
		public:
			~VirtualFilesystem();

			auto Mount(std::string_view mount_point, IFilesystem* filesystem) -> void;
			auto Unmount(std::string_view mount_point) -> void;

			auto OpenFile(std::string_view path, FileModeFlags mode) -> IFile*;
			auto FileSize(std::string_view path) -> size_t;
			auto ReadFile(std::string_view path, std::string& buffer) -> bool;
			auto ReadFile(std::string_view path, std::vector<uint8_t>& buffer) -> bool;
			auto WriteFile(std::string_view path, std::string_view buffer) -> bool;
			auto WriteFile(std::string_view path, std::span<const uint8_t> buffer) -> bool;

			auto CreateDirectories(std::string_view path) -> bool;
			auto CreateDirectory(std::string_view path) -> bool;

			auto Exists(std::string_view path) -> bool;
			auto IsDirectory(std::string_view path) -> bool;
			auto IsFile(std::string_view path) -> bool;

			auto Remove(std::string_view path) -> bool;

			auto Walk(std::string_view path, bool recursive = false) -> IDirectoryIterator*;
		private:
			auto GetFilesystem(std::string_view path) -> IFilesystem*;

			std::unordered_map<std::string_view, IFilesystem*> filesystems;
		};
	}
}