#include "GPakFileSystem.h"
#include "GPakInputFile.h"
#include "GPakIterator.h"

#include <gpak.h>
#include <filesystem_tree.h>

using namespace engine::filesystem;

GPakFileSystem::GPakFileSystem(const char* fspath)
{
	Create(fspath);
}

GPakFileSystem::~GPakFileSystem()
{
	if (m_pHandle)
		gpak_close(m_pHandle);
}

bool GPakFileSystem::Create(const char* fspath)
{
	m_pHandle = gpak_open(fspath, GPAK_MODE_READ_ONLY);

	return m_pHandle;
}

bool GPakFileSystem::Exists(const char* path)
{
	if (auto* _entry = gpak_get_root(m_pHandle))
	{
		auto* _found_dir = filesystem_tree_find_directory(_entry, path);
		auto* _found_file = filesystem_tree_find_file(_entry, path);
		return _found_dir || _found_file;
	}

	return false;
}

size_t GPakFileSystem::FileSize(const char* path)
{
	if (auto* _entry = gpak_get_root(m_pHandle))
	{
		if (auto* _file = filesystem_tree_find_file(_entry, path))
			return _file->entry_.uncompressed_size_;
	}

	return 0ull;
}

bool GPakFileSystem::IsDirectory(const char* path)
{
	if (auto* _entry = gpak_get_root(m_pHandle))
		return filesystem_tree_find_directory(_entry, path);

	return false;
}

bool GPakFileSystem::IsFile(const char* path)
{
	if (auto* _entry = gpak_get_root(m_pHandle))
		return filesystem_tree_find_file(_entry, path);

	return false;
}

std::unique_ptr<IInputFileInterface> GPakFileSystem::OpenFile(const char* path)
{
	if (!IsFile(path))
		return nullptr;

	return std::make_unique<GPakInputFile>(m_pHandle, path);
}

std::unique_ptr<IDirectoryIterator> GPakFileSystem::Walk(const char* path, bool recursive)
{
	return std::make_unique<GPakIterator>(m_pHandle, path, recursive);
}

EFilesystemModeFlags GPakFileSystem::GetFilesystemMode() const
{
	return static_cast<uint32_t>(EFilesystemModeFlagBits::eRead);
}