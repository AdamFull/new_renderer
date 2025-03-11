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
			fclose(file_handle);
		}

		auto NativeFile::Seek(uint64_t offset, SeekOrigin origin) -> uint64_t
		{
			return fseek(file_handle, offset, (int32_t)origin);
		}

		auto NativeFile::Tell() -> uint64_t
		{
			return ftell(file_handle);
		}

		auto NativeFile::Read(void* buffer, uint64_t size) -> uint64_t
		{
			return fread(buffer, 1, size, file_handle);
		}

		auto NativeFile::Write(const void* buffer, uint64_t size) -> uint64_t
		{
			return fwrite(buffer, 1, size, file_handle);
		}
	}
}