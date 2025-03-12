#pragma once

#include <string_view>
#include "fs_file.h"

namespace edge
{
	namespace fs
	{
		class IDirectoryIterator;

		class IFilesystem
		{
		public:
			virtual ~IFilesystem() = default;

			virtual auto OpenFile(std::string_view path, FileModeFlags mode) -> IFile* = 0;
			virtual auto FileSize(std::string_view path) -> size_t = 0;

			virtual auto CreateDirectories(std::string_view path) -> bool = 0;
			virtual auto CreateDirectory(std::string_view path) -> bool = 0;

			virtual auto Exists(std::string_view path) -> bool = 0;
			virtual auto IsDirectory(std::string_view path) -> bool = 0;
			virtual auto IsFile(std::string_view path) -> bool = 0;

			virtual auto Remove(std::string_view path) -> bool = 0;

			virtual auto Walk(std::string_view path, bool recursive = false) -> IDirectoryIterator* = 0;
		};
	}
}