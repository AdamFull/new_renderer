#include "ZipFileSystem.h"
#include "ZipInputFile.h"
#include "ZipIterator.h"

#include <zip.h>

namespace edge
{
	namespace filesystem
	{
		ZipFileSystem::ZipFileSystem(std::string_view fspath, std::string_view password)
		{
			Create(fspath);
			m_srPassword = password;
		}

		ZipFileSystem::~ZipFileSystem()
		{
			if (m_pArchive)
				zip_close(m_pArchive);
		}


		bool ZipFileSystem::Create(std::string_view fspath)
		{
			if (m_pArchive)
				return false;

			int error_code = 0;
			m_pArchive = zip_open(fspath.data(), ZIP_RDONLY, &error_code);

			if (!m_pArchive)
			{
				char error_message[256];
				zip_error_to_str(error_message, sizeof(error_message), error_code, errno);
				return false;
			}

			return true;
		}

		bool ZipFileSystem::Exists(std::string_view path)
		{
			if (!m_pArchive)
				return false;

			zip_stat_t stat_info;
			if (zip_stat(m_pArchive, path.data(), ZIP_FL_NOCASE, &stat_info) == 0)
				return stat_info.valid & ZIP_STAT_NAME;

			return false;
		}

		size_t ZipFileSystem::FileSize(std::string_view path)
		{
			if (!m_pArchive)
				return 0;

			zip_stat_t stat_info;
			if (zip_stat(m_pArchive, path.data(), 0, &stat_info) == 0)
				return static_cast<size_t>(stat_info.size);

			return 0ull;
		}

		bool ZipFileSystem::IsDirectory(std::string_view path)
		{
			if (!m_pArchive)
				return false;

			zip_stat_t stat_info;
			if (zip_stat(m_pArchive, path.data(), ZIP_FL_NOCASE, &stat_info) == 0)
				return (stat_info.valid & ZIP_STAT_NAME) && (stat_info.name[strlen(stat_info.name) - 1] == '/');

			return false;
		}

		bool ZipFileSystem::IsFile(std::string_view path)
		{
			if (!m_pArchive)
				return false;

			zip_stat_t stat_info;
			if (zip_stat(m_pArchive, path.data(), ZIP_FL_NOCASE, &stat_info) == 0)
				return (stat_info.valid & ZIP_STAT_NAME) && (stat_info.name[strlen(stat_info.name) - 1] != '/');

			return false;
		}

		std::unique_ptr<IInputFileInterface> ZipFileSystem::OpenFile(std::string_view path)
		{
			if (!IsFile(path))
				return nullptr;

			return std::make_unique<ZipInputFile>(m_pArchive, path, m_srPassword);
		}

		std::unique_ptr<IDirectoryIterator>  ZipFileSystem::Walk(std::string_view path, bool recursive)
		{
			return std::make_unique<ZipIterator>(m_pArchive, path, recursive);;
		}

		EFilesystemModeFlags ZipFileSystem::GetFilesystemMode() const
		{
			return static_cast<uint32_t>(EFilesystemModeFlagBits::eRead);
		}
	}
}