#include "NativeFileSystem.h"
#include "NativeInputFile.h"
#include "NativeOutputFile.h"
#include "NativeIterator.h"

using namespace engine::filesystem;

NativeFileSystem::NativeFileSystem(std::string_view fspath)
{
	Create(fspath);
}

bool NativeFileSystem::Create(std::string_view fspath)
{
	if (fspath.empty())
		throw std::runtime_error("Invalid path for native fs.");

	m_fsRoot = std::filesystem::weakly_canonical(std::filesystem::path(fspath));
	return true;
}

void NativeFileSystem::Copy(std::string_view from, std::string_view to)
{
	auto fromp = std::filesystem::weakly_canonical(m_fsRoot / from);
	auto top = std::filesystem::weakly_canonical(m_fsRoot / to);

	std::filesystem::copy(fromp, top);
}

bool NativeFileSystem::CreateDirectories(std::string_view path)
{
	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);
	return std::filesystem::create_directories(pathp);
}

bool NativeFileSystem::CreateDirectory(std::string_view path)
{
	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);
	return std::filesystem::create_directory(pathp);
}

bool NativeFileSystem::Exists(std::string_view path)
{
	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);
	return std::filesystem::exists(pathp);
}

size_t NativeFileSystem::FileSize(std::string_view path)
{
	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);
	return std::filesystem::file_size(pathp);
}

bool NativeFileSystem::IsDirectory(std::string_view path)
{
	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);
	return std::filesystem::is_directory(pathp);
}

bool NativeFileSystem::IsFile(std::string_view path)
{
	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);
	return std::filesystem::is_regular_file(pathp);
}

bool NativeFileSystem::Remove(std::string_view path)
{
	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);
	return std::filesystem::remove(pathp);
}

void NativeFileSystem::Rename(std::string_view original, std::string_view newfile)
{
	auto fromp = std::filesystem::weakly_canonical(m_fsRoot / original);
	auto top = std::filesystem::weakly_canonical(m_fsRoot / newfile);
	std::filesystem::rename(fromp, top);
}

std::unique_ptr<IOutputFileInterface> NativeFileSystem::CreateFile(std::string_view path)
{
	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);
	return std::make_unique<NativeOutputFile>(pathp.string());
}

std::unique_ptr<IInputFileInterface> NativeFileSystem::OpenFile(std::string_view path)
{
	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);

	if (!IsFile(pathp.string().c_str()))
		return nullptr;

	return std::make_unique<CNativeInputFile>(pathp.string());
}

std::unique_ptr<IDirectoryIterator> NativeFileSystem::Walk(std::string_view path, bool recursive)
{
	std::unique_ptr<IDirectoryIterator> iterator{ nullptr };

	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);
	if (recursive)
		iterator = std::make_unique<NativeIterator<std::filesystem::recursive_directory_iterator>>(pathp.string(), m_fsRoot.string());
	else
		iterator = std::make_unique<NativeIterator<std::filesystem::directory_iterator>>(pathp.string(), m_fsRoot.string());

	return iterator;
}

EFilesystemModeFlags NativeFileSystem::GetFilesystemMode() const
{
	return static_cast<uint32_t>(EFilesystemModeFlagBits::eRead) | static_cast<uint32_t>(EFilesystemModeFlagBits::eWrite);
}