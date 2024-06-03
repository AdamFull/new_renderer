#include "GPakFileSystem.h"
#include "GPakInputFile.h"
#include "GPakIterator.h"

#include <gpak.h>
#include <filesystem_tree.h>

using namespace engine::filesystem;

CGPakFileSystem::CGPakFileSystem(const std::string& fspath)
{
	Create(fspath);
}

CGPakFileSystem::~CGPakFileSystem()
{
	if (m_pHandle)
		gpak_close(m_pHandle);
}

bool CGPakFileSystem::Create(const std::string& fspath)
{
	m_pHandle = gpak_open(fspath.c_str(), GPAK_MODE_READ_ONLY);

	return m_pHandle;
}

bool CGPakFileSystem::Exists(const std::string& path)
{
	if (auto* _entry = gpak_get_root(m_pHandle))
	{
		auto* _found_dir = filesystem_tree_find_directory(_entry, path.c_str());
		auto* _found_file = filesystem_tree_find_file(_entry, path.c_str());
		return _found_dir || _found_file;
	}

	return false;
}

size_t CGPakFileSystem::FileSize(const std::string& path)
{
	if (auto* _entry = gpak_get_root(m_pHandle))
	{
		if (auto* _file = filesystem_tree_find_file(_entry, path.c_str()))
			return _file->entry_.uncompressed_size_;
	}

	return 0ull;
}

bool CGPakFileSystem::IsDirectory(const std::string& path)
{
	if (auto* _entry = gpak_get_root(m_pHandle))
		return filesystem_tree_find_directory(_entry, path.c_str());

	return false;
}

bool CGPakFileSystem::IsFile(const std::string& path)
{
	if (auto* _entry = gpak_get_root(m_pHandle))
		return filesystem_tree_find_file(_entry, path.c_str());

	return false;
}

std::unique_ptr<IInputFileInterface> CGPakFileSystem::OpenFile(const std::string& path)
{
	if (!IsFile(path))
		return nullptr;

	return std::make_unique<CGPakInputFile>(m_pHandle, path);
}

std::unique_ptr<IDirectoryIterator> CGPakFileSystem::Walk(const std::string& path, bool recursive)
{
	return std::make_unique<CGPakIterator>(m_pHandle, path, recursive);
}

EFilesystemModeFlags CGPakFileSystem::GetFilesystemMode() const
{
	return static_cast<uint32_t>(EFilesystemModeFlagBits::eRead);
}