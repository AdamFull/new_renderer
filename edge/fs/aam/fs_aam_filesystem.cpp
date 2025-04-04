#include "fs_aam_filesystem.h"

#include "fs_aam_directory_iterator.h"
#include "fs_aam_file.h"

#include "../../util/Logger.h"

#include <android/asset_manager.h>

namespace edge::fs {
	AAMFilesystem::AAMFilesystem(AAssetManager* asset_manager) :
		asset_manager{ asset_manager } {
	}

	AAMFilesystem::~AAMFilesystem() {
	}

	auto AAMFilesystem::open_file(std::string_view path, FileModeFlags mode) -> IFile* {
		std::lock_guard<std::mutex> lock(mutex);
		return new AAMFile(asset_manager, path, mode);
	}

	auto AAMFilesystem::file_size(std::string_view path) -> size_t {
		std::lock_guard<std::mutex> lock(mutex);

        AAsset* asset = AAssetManager_open(asset_manager, std::string(path).c_str(), AASSET_MODE_STREAMING);
        if (!asset) return 0;
        off_t length = AAsset_getLength(asset);
        AAsset_close(asset);
        return static_cast<size_t>(length);
	}

	auto AAMFilesystem::create_directories(std::string_view path) -> bool {
		assert(false && "NOT IMPLEMENTED");
		return false;
	}

	auto AAMFilesystem::create_directory(std::string_view path) -> bool {
		assert(false && "NOT IMPLEMENTED");
		return false;
	}

	auto AAMFilesystem::exists(std::string_view path) -> bool {
		std::lock_guard<std::mutex> lock(mutex);

        if (is_file(path) || is_directory(path)) {
            return true;
        }

		return false;
	}

	auto AAMFilesystem::is_directory(std::string_view path) -> bool {
		std::lock_guard<std::mutex> lock(mutex);
        AAssetDir* dir = AAssetManager_openDir(asset_manager, path.data());
        if (!dir) return false;
        const char* name = AAssetDir_getNextFileName(dir);
        AAssetDir_close(dir);
        return name != nullptr;
	}

	auto AAMFilesystem::is_file(std::string_view path) -> bool {
		std::lock_guard<std::mutex> lock(mutex);
        AAsset* asset = AAssetManager_open(asset_manager, path.data(), AASSET_MODE_STREAMING);
        if (!asset) return false;
        AAsset_close(asset);
        return true;
	}

	auto AAMFilesystem::remove(std::string_view path) -> bool {
		assert(false && "NOT IMPLEMENTED");
		return false;
	}

	auto AAMFilesystem::walk(std::string_view path, bool recursive) -> IDirectoryIterator* {
		std::lock_guard<std::mutex> lock(mutex);
		return new AAMDirectoryIterator(asset_manager, path, recursive);
	}
}