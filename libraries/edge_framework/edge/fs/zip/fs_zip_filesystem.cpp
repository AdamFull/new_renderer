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

		auto ZipFilesystem::open_file(std::string_view path, FileModeFlags mode) -> IFile*
		{
			std::lock_guard<std::mutex> lock(mutex);
			return new ZipFile(zip_handle, path, mode);
		}

		auto ZipFilesystem::file_size(std::string_view path) -> size_t
		{
			std::lock_guard<std::mutex> lock(mutex);
			zip_stat_t st;
			if (zip_stat(zip_handle, path.data(), ZIP_FL_ENC_GUESS | ZIP_FL_NOCASE, &st) == 0)
				return st.size;

			return 0ull;
		}

		auto ZipFilesystem::create_directories(std::string_view path) -> bool
		{
			assert(false && "NOT IMPLEMENTED");
			return false;
		}

		auto ZipFilesystem::create_directory(std::string_view path) -> bool
		{
			assert(false && "NOT IMPLEMENTED");
			return false;
		}

		auto ZipFilesystem::exists(std::string_view path) -> bool
		{
			std::lock_guard<std::mutex> lock(mutex);

			if (!zip_handle)
				return false;

			return zip_name_locate(zip_handle, path.data(), ZIP_FL_ENC_GUESS | ZIP_FL_NOCASE) >= 0;
		}

		auto ZipFilesystem::is_directory(std::string_view path) -> bool
		{
			std::lock_guard<std::mutex> lock(mutex);

			if (!zip_handle)
				return false;

			zip_stat_t st;
			if (zip_stat(zip_handle, path.data(), ZIP_FL_ENC_GUESS | ZIP_FL_NOCASE, &st) == 0)
				return (st.valid & ZIP_STAT_NAME) && (st.name[strlen(st.name) - 1] == '/');

			return false;
		}

		auto ZipFilesystem::is_file(std::string_view path) -> bool
		{
			std::lock_guard<std::mutex> lock(mutex);

			if (!zip_handle)
				return false;

			zip_stat_t st;
			if (zip_stat(zip_handle, path.data(), ZIP_FL_ENC_GUESS | ZIP_FL_NOCASE, &st) == 0)
				return (st.valid & ZIP_STAT_NAME) && (st.name[strlen(st.name) - 1] != '/');

			return false;
		}

		auto ZipFilesystem::remove(std::string_view path) -> bool
		{
			assert(false && "NOT IMPLEMENTED");
			return false;
		}

		auto ZipFilesystem::walk(std::string_view path, bool recursive) -> IDirectoryIterator*
		{
			std::lock_guard<std::mutex> lock(mutex);
			return new ZipDirectoryIterator(zip_handle, path, recursive);
		}
	}
}