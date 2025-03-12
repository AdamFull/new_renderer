#pragma once

#include "../fs_filesystem.h"
#include <filesystem>

namespace edge
{
	namespace fs
	{
		class NativeFilesystem final : public IFilesystem
		{
		public:
			NativeFilesystem(std::string_view root);
			~NativeFilesystem();

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
			auto GetPath(std::string_view local_path) -> std::filesystem::path;
			std::filesystem::path fs_root;
		};
	}
}