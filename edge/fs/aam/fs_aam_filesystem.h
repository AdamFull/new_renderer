#pragma once

#include "../fs_filesystem.h"

#include <mutex>

struct AAssetManager;

namespace edge::fs
{
	class AAMFilesystem final : public IFilesystem {
	public:
		AAMFilesystem(AAssetManager* asset_manager);
		~AAMFilesystem();

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
		AAssetManager* asset_manager{ nullptr };

		std::mutex mutex;
	};
}