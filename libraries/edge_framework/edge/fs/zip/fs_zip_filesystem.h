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

			auto OpenFile(std::string_view path, FileModeFlags mode) -> IFile* override;
			auto FileSize(std::string_view path) -> size_t override;

			auto CreateDirectories(std::string_view path) -> bool override;
			auto CreateDirectory(std::string_view path) -> bool override;

			auto Exists(std::string_view path) -> bool override;
			auto IsDirectory(std::string_view path) -> bool override;
			auto IsFile(std::string_view path) -> bool override;

			auto Remove(std::string_view path) -> bool override;

			auto Walk(std::string_view path, bool recursive = false) -> IDirectoryIterator* override;
		private:
			zip_t* zip_handle{ nullptr };

			std::mutex mutex;
		};
	}
}