#include "fs_aam_file.h"

#include "../../util/Logger.h"

#include <android/asset_manager.h>

namespace edge::fs {
	AAMFile::AAMFile(AAssetManager* asset_manager, std::string_view path, FileModeFlags mode_flags) :
		asset_manager{ asset_manager } {
        asset = AAssetManager_open(asset_manager, std::string(path).c_str(), AASSET_MODE_STREAMING);
	}

	AAMFile::~AAMFile() {
		close();
	}

	auto AAMFile::is_open() -> bool {
		return asset != nullptr;
	}

	auto AAMFile::close() -> void {
		std::lock_guard<std::mutex> lock(mutex);
        if(asset) {
            AAsset_close(asset);
            asset = nullptr;
        }
	}

	auto AAMFile::size() -> uint64_t {
		return AAsset_getLength64(asset);
	}

	auto AAMFile::seek(uint64_t offset, SeekOrigin origin) -> int64_t
	{
		std::lock_guard<std::mutex> lock(mutex);
		return AAsset_seek64(asset, (off64_t)offset, (int)origin);
	}

	auto AAMFile::tell() -> int64_t {
		std::lock_guard<std::mutex> lock(mutex);
        return AAsset_seek(asset, 0, SEEK_CUR);
	}

	auto AAMFile::read(void* buffer, uint64_t size) -> int64_t {
		std::lock_guard<std::mutex> lock(mutex);
		return AAsset_read(asset, buffer, size);
	}

	auto AAMFile::write(const void* buffer, uint64_t size) -> int64_t {
		assert(false && "NOT IMPLEMENTED");
		return -1;
	}
}