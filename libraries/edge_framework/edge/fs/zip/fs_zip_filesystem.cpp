#include "fs_zip_filesystem.h"

#include "fs_zip_derectory_iterator.h"
#include "fs_zip_file.h"

#include "../../util/Logger.h"

#include <zip.h>

#ifdef CreateDirectory
#undef CreateDirectory
#endif

namespace edge
{
	namespace fs
	{
		ZipFilesystem::ZipFilesystem(std::string_view root, std::string_view password)
		{
			int32_t error_code = 0;
			zip_handle = zip_open(root.data(), ZIP_RDONLY, &error_code);

			if (!zip_handle)
			{
				char error_message[256];
				zip_error_to_str(error_message, sizeof(error_message), error_code, errno);

				EDGE_LOGE("[ZipFilesystem] Failed to open archive: {}.", error_message);
				return;
			}

			if (!password.empty())
			{
				zip_set_default_password(zip_handle, password.data());
			}
		}

		ZipFilesystem::~ZipFilesystem()
		{
			if (zip_handle)
			{
				zip_close(zip_handle);
			}
		}

		auto ZipFilesystem::OpenFile(std::string_view path, FileModeFlags mode) -> IFile*
		{
			return new ZipFile(zip_handle, path, mode);
		}

		auto ZipFilesystem::CreateDirectories(std::string_view path) -> bool
		{
			assert(false && "NOT IMPLEMENTED");
			return false;
		}

		auto ZipFilesystem::CreateDirectory(std::string_view path) -> bool
		{
			assert(false && "NOT IMPLEMENTED");
			return false;
		}

		auto ZipFilesystem::Exists(std::string_view path) -> bool
		{
			if (!zip_handle)
				return false;

			zip_int64_t idx = zip_name_locate(zip_handle, path.data(), ZIP_FL_ENC_GUESS);
			return idx >= 0;
		}

		auto ZipFilesystem::IsDirectory(std::string_view path) -> bool
		{
			if (!zip_handle)
				return false;

			zip_stat_t stat_info;
			if (zip_stat(zip_handle, path.data(), ZIP_FL_NOCASE, &stat_info) == 0)
				return (stat_info.valid & ZIP_STAT_NAME) && (stat_info.name[strlen(stat_info.name) - 1] == '/');

			return false;
		}

		auto ZipFilesystem::IsFile(std::string_view path) -> bool
		{
			if (!zip_handle)
				return false;

			zip_stat_t stat_info;
			if (zip_stat(zip_handle, path.data(), ZIP_FL_NOCASE, &stat_info) == 0)
				return (stat_info.valid & ZIP_STAT_NAME) && (stat_info.name[strlen(stat_info.name) - 1] != '/');

			return false;
		}

		auto ZipFilesystem::Remove(std::string_view path) -> bool
		{
			assert(false && "NOT IMPLEMENTED");
			return false;
		}

		auto ZipFilesystem::Walk(std::string_view path, bool recursive) -> IDirectoryIterator*
		{
			return new ZipDirectoryIterator(zip_handle, path, recursive);
		}
	}
}