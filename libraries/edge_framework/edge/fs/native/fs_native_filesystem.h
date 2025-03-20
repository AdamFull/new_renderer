#pragma once

#include "../fs_filesystem.h"
#include <filesystem>

namespace edge::fs
{
	class NativeFilesystem final : public IFilesystem
	{
	public:
		NativeFilesystem(std::string_view root);
		~NativeFilesystem();

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
		auto get_path(std::string_view local_path) -> std::filesystem::path;
		std::filesystem::path fs_root;
	};
}