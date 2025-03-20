#pragma once

#include "../fs_file.h"

#include <mutex>

typedef struct zip zip_t;
typedef struct zip_file zip_file_t;
typedef struct zip_stat zip_stat_t;

namespace edge::fs
{
	class ZipFile final : public IFile {
	public:
		ZipFile(zip_t* zip, std::string_view path, FileModeFlags mode_flags);
		~ZipFile() override;

		[[nodiscard]] auto is_open() -> bool override;
		auto close() -> void override;

		[[nodiscard]] auto size() -> uint64_t override;
		auto seek(uint64_t offset, SeekOrigin origin) -> int64_t override;
		[[nodiscard]] auto tell() -> int64_t override;
		auto read(void* buffer, uint64_t size) -> int64_t override;
		auto write(const void* buffer, uint64_t size) -> int64_t override;
	private:
		zip_t* zip_handle{ nullptr };
		zip_file_t* file_handle{ nullptr };
		zip_stat_t* file_stat{ nullptr };

		std::mutex mutex;
	};
}