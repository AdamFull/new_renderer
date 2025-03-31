#pragma once

#include "../fs_file.h"
#include <string_view>
#include <cstdio>

namespace edge::fs
{
	class NativeFile final : public IFile {
	public:
		NativeFile(std::string_view path, FileModeFlags mode_flags);
		~NativeFile() override;

		[[nodiscard]] auto is_open() -> bool override;
		auto close() -> void override;

		[[nodiscard]] auto size() -> uint64_t override;
		auto seek(uint64_t offset, SeekOrigin origin) -> int64_t override;
		[[nodiscard]] auto tell() -> int64_t override;
		auto read(void* buffer, uint64_t size) -> int64_t override;
		auto write(const void* buffer, uint64_t size) -> int64_t override;
	private:
		FILE* file_handle{ nullptr };
	};
}