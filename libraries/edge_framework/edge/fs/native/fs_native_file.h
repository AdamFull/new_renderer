#pragma once

#include "../fs_file.h"
#include <string_view>
#include <cstdio>

namespace edge
{
	namespace fs
	{
		class NativeFile final : public IFile
		{
		public:
			NativeFile(std::string_view path, FileModeFlags mode_flags);
			~NativeFile() override;

			[[nodiscard]] auto IsOpen() -> bool override;
			auto Close() -> void override;

			[[nodiscard]] auto Size() -> uint64_t override;

			[[nodiscard]] auto Seek(uint64_t offset, SeekOrigin origin) -> uint64_t override;
			[[nodiscard]] auto Tell() -> uint64_t override;
			[[nodiscard]] auto Read(void* buffer, uint64_t size) -> uint64_t override;
			[[nodiscard]] auto Write(const void* buffer, uint64_t size) -> uint64_t override;
		private:
			FILE* file_handle{ nullptr };
		};
	}
}