#pragma once

#include "../fs_filesystem.h"

#include <mutex>

typedef struct zip zip_t;

namespace edge
{
	namespace fs
	{
		class ZipFilesystem final : public IFilesystem
		{
		public:
			ZipFilesystem(std::string_view root, std::string_view password = {});
			~ZipFilesystem();

			auto open_file(std::string_view path, FileModeFlags mode) -> IFile* override;
			auto file_size(std::string_view path) -> size_t override;

			auto create_directories(std::string_view path) -> bool override;
			auto create_directory(std::string_view path) -> bool override;

			auto exists(std::string_view path) -> bool override;
			auto is_directory(std::string_view path) -> bool override;
			auto is_file(std::string_view path) -> bool override;

			auto remove(std::string_view path) -> bool override;

			auto walk(std::string_view path, bool recursive = false) -> IDirectoryIterator* override;
		private:
			zip_t* zip_handle{ nullptr };

			std::mutex mutex;
		};
	}
}