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

			virtual auto open_file(std::string_view path, FileModeFlags mode) -> IFile* = 0;
			virtual auto file_size(std::string_view path) -> size_t = 0;

			virtual auto create_directories(std::string_view path) -> bool = 0;
			virtual auto create_directory(std::string_view path) -> bool = 0;

			virtual auto exists(std::string_view path) -> bool = 0;
			virtual auto is_directory(std::string_view path) -> bool = 0;
			virtual auto is_file(std::string_view path) -> bool = 0;

			virtual auto remove(std::string_view path) -> bool = 0;

			virtual auto walk(std::string_view path, bool recursive = false) -> IDirectoryIterator* = 0;
		};
	}
}