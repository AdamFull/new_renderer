#include "fs_native_file.h"

namespace edge
{
	namespace fs
	{
		NativeFile::NativeFile(std::string_view path, FileModeFlags mode_flags)
		{
			const char* fmode = "";
			if (mode_flags & FileModeFlag::eRead && mode_flags & FileModeFlag::eWrite) 
				fmode = "r+";
			else if (mode_flags & FileModeFlag::eRead) 
				fmode = "r";
			else if (mode_flags & FileModeFlag::eWrite) 
			{
				if (mode_flags & FileModeFlag::eAppend)
					fmode = "a";
				else
					fmode = "w";
			}

			char buffer[4];
			if (mode_flags & FileModeFlag::eBin)
			{
				snprintf(buffer, sizeof(buffer), "%sb", fmode);
				fmode = buffer;
			}

			file_handle = fopen(path.data(), fmode);
		}

		NativeFile::~NativeFile()
		{
			Close();
		}

		auto NativeFile::IsOpen() -> bool
		{
			return (file_handle);
		}
		
		auto NativeFile::Close() -> void
		{
			if (file_handle)
			{
				fclose(file_handle);
				file_handle = nullptr;
			}
		}

		auto NativeFile::Size() -> uint64_t
		{
			auto cursor = ftell(file_handle);
			fseek(file_handle, 0, SEEK_END);
			auto size = ftell(file_handle);
			fseek(file_handle, cursor, SEEK_SET);
			return size;
		}

		auto NativeFile::Seek(uint64_t offset, SeekOrigin origin) -> int64_t
		{
			if (fseek(file_handle, static_cast<long>(offset), static_cast<int>(origin)) != 0)
				return -1;
			return ftell(file_handle);
		}

		auto NativeFile::Tell() -> int64_t
		{
			return ftell(file_handle);
		}

		auto NativeFile::Read(void* buffer, uint64_t size) -> int64_t
		{
			return fread(buffer, 1, size, file_handle);
		}

		auto NativeFile::Write(const void* buffer, uint64_t size) -> int64_t
		{
			return fwrite(buffer, 1, size, file_handle);
		}
	}
}