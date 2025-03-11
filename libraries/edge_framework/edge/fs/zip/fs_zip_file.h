#pragma once

#include "../fs_file.h"

typedef struct zip zip_t;
typedef struct zip_file zip_file_t;

namespace edge
{
	namespace fs
	{
		class ZipFile final : public IFile
		{
		public:
			ZipFile(zip_t* zip, std::string_view path, FileModeFlags mode_flags);
			~ZipFile() override;

			[[nodiscard]] auto IsOpen() -> bool override;
			auto Close() -> void override;

			[[nodiscard]] auto Seek(uint64_t offset, SeekOrigin origin) -> uint64_t override;
			[[nodiscard]] auto Tell() -> uint64_t override;
			[[nodiscard]] auto Read(void* buffer, uint64_t size) -> uint64_t override;
			[[nodiscard]] auto Write(const void* buffer, uint64_t size) -> uint64_t override;
		private:
			zip_t* zip_handle{ nullptr };
			zip_file_t* file_handle{ nullptr };
		};
	}
}