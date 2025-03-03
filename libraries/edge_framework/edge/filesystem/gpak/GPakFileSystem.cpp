#include "GPakFileSystem.h"
#include "GPakInputFile.h"
#include "GPakIterator.h"

#include <gpak.h>
#include <filesystem_tree.h>

namespace edge
{
	namespace filesystem
	{
		GPakFileSystem::GPakFileSystem(std::string_view fspath)
		{
			Create(fspath);
		}

		GPakFileSystem::~GPakFileSystem()
		{
			if (m_pHandle)
				gpak_close(m_pHandle);
		}

		bool GPakFileSystem::Create(std::string_view fspath)
		{
			m_pHandle = gpak_open(fspath.data(), GPAK_MODE_READ_ONLY);

			return m_pHandle;
		}

		bool GPakFileSystem::Exists(std::string_view path)
		{
			if (auto* _entry = gpak_get_root(m_pHandle))
			{
				auto* _found_dir = filesystem_tree_find_directory(_entry, path.data());
				auto* _found_file = filesystem_tree_find_file(_entry, path.data());
				return _found_dir || _found_file;
			}

			return false;
		}

		size_t GPakFileSystem::FileSize(std::string_view path)
		{
			if (auto* _entry = gpak_get_root(m_pHandle))
			{
				if (auto* _file = filesystem_tree_find_file(_entry, path.data()))
					return _file->entry_.uncompressed_size_;
			}

			return 0ull;
		}

		bool GPakFileSystem::IsDirectory(std::string_view path)
		{
			if (auto* _entry = gpak_get_root(m_pHandle))
				return filesystem_tree_find_directory(_entry, path.data());

			return false;
		}

		bool GPakFileSystem::IsFile(std::string_view path)
		{
			if (auto* _entry = gpak_get_root(m_pHandle))
				return filesystem_tree_find_file(_entry, path.data());

			return false;
		}

		std::unique_ptr<IInputFileInterface> GPakFileSystem::OpenFile(std::string_view path)
		{
			if (!IsFile(path))
				return nullptr;

			return std::make_unique<GPakInputFile>(m_pHandle, path);
		}

		std::unique_ptr<IDirectoryIterator> GPakFileSystem::Walk(std::string_view path, bool recursive)
		{
			return std::make_unique<GPakIterator>(m_pHandle, path, recursive);
		}
	}
}