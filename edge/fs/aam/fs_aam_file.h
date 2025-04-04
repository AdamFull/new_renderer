#pragma once

#include "../fs_file.h"

#include <mutex>

struct AAssetManager;
struct AAsset;

namespace edge::fs {
	class AAMFile final : public IFile {
	public:
		AAMFile(AAssetManager* asset_manager, std::string_view path, FileModeFlags mode_flags);
		~AAMFile() override;

		[[nodiscard]] auto is_open() -> bool override;
		auto close() -> void override;

		[[nodiscard]] auto size() -> uint64_t override;
		auto seek(uint64_t offset, SeekOrigin origin) -> int64_t override;
		[[nodiscard]] auto tell() -> int64_t override;
		auto read(void* buffer, uint64_t size) -> int64_t override;
		auto write(const void* buffer, uint64_t size) -> int64_t override;
	private:
		AAssetManager* asset_manager{ nullptr };
		AAsset* asset{ nullptr };

		std::mutex mutex;
	};
}