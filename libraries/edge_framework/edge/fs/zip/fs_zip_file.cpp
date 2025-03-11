#include "fs_zip_file.h"

#include <zip.h>

namespace edge
{
	namespace fs
	{
		ZipFile::ZipFile(zip_t* zip, std::string_view path, FileModeFlags mode_flags) :
			zip_handle{ zip }
		{
			file_handle = zip_fopen(zip_handle, path.data(), 0);
			//zip_fopen_encrypted(zip_handle, path.data(), 0, nullptr);
			// TODO: Add encryption support
		}

		ZipFile::~ZipFile()
		{
			Close();
		}

		auto ZipFile::IsOpen() -> bool
		{
			return (file_handle);
		}

		auto ZipFile::Close() -> void
		{
			if (file_handle)
			{
				zip_fclose(file_handle);
				file_handle = nullptr;
			}
		}

		auto ZipFile::Seek(uint64_t offset, SeekOrigin origin) -> uint64_t
		{
			if (!file_handle)
			{
				return 0ull;
			}

			return zip_fseek(file_handle, offset, (int32_t)origin);
		}

		auto ZipFile::Tell() -> uint64_t
		{
			if (!file_handle)
			{
				return 0ull;
			}

			return zip_ftell(file_handle);
		}

		auto ZipFile::Read(void* buffer, uint64_t size) -> uint64_t
		{
			if (!file_handle)
			{
				return 0ull;
			}

			return zip_fread(file_handle, buffer, size);
		}

		auto ZipFile::Write(const void* buffer, uint64_t size) -> uint64_t 
		{
			return 0ull;
		}
	}
}