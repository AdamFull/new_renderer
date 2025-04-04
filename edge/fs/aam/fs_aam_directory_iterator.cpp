#include "fs_aam_directory_iterator.h"

#include "../../util/Logger.h"

#include <android/asset_manager.h>

namespace edge::fs {
	AAMDirectoryIterator::AAMDirectoryIterator(AAssetManager* asset_manager, std::string_view path, bool recursive) :
		asset_manager{ asset_manager },
		recursive{ recursive } {

        root = std::string(path);
        if (!root.empty() && root.back() != '/')
            root.push_back('/');

        current_entry_path = root;

        current_dir = AAssetManager_openDir(asset_manager, root.c_str());
        if(!current_dir) {
            EDGE_LOGE("[AAM Directory Iterator] Failed to open entry: \"{}\". Not found.", path);
        }
	}

	AAMDirectoryIterator::~AAMDirectoryIterator() {
        if(current_dir) {
            AAssetDir_close(current_dir);
            current_dir = nullptr;
        }

        while (!dir_stack.empty()) {
            auto pair = dir_stack.top();
            dir_stack.pop();
            if (pair.second) {
                AAssetDir_close(pair.second);
            }
        }
	}

	auto AAMDirectoryIterator::end() const noexcept -> bool {
		return current_dir == nullptr;
	}

	auto AAMDirectoryIterator::next() -> void {
		std::lock_guard<std::mutex> lock(mutex);

        if (!current_dir)
            return;

        const char* entry = nullptr;
        while(true) {
            entry = AAssetDir_getNextFileName(current_dir);
            if (entry) {
                std::string full_path = root + entry;
                if (recursive) {
                    // Attempt to open this entry as a directory.
                    AAssetDir* sub_dir = AAssetManager_openDir(asset_manager, full_path.c_str());
                    if (sub_dir != nullptr) {
                        current_entry_path = full_path;
                        dir_stack.emplace( root, current_dir );
                        current_dir = sub_dir;
                        // Ensure the new prefix ends with a slash.
                        if (full_path.back() != '/')
                            root = full_path + "/";
                        else
                            root = full_path;
                        break; // Yield this directory entry.
                    }
                }

                current_entry_path = full_path;
                break;
            }
            else {
                AAssetDir_close(current_dir);
                if (!dir_stack.empty()) {
                    // Pop the previous directory context and continue iterating.
                    auto ctx = dir_stack.top();
                    dir_stack.pop();
                    current_dir = ctx.second;
                    root = ctx.first;
                    // Loop to get the next entry from the resumed directory.
                    continue;
                } else {
                    // No more directories to process.
                    current_dir = nullptr;
                    break;
                }
            }
        }
	}

	auto AAMDirectoryIterator::value() const noexcept -> std::string_view {
		return current_entry_path;
	}
}