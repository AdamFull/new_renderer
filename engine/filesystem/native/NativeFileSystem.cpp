#include "NativeFileSystem.h"
#include "NativeInputFile.h"
#include "NativeOutputFile.h"
#include "NativeIterator.h"

using namespace engine::filesystem;

CNativeFileSystem::CNativeFileSystem(const std::string& fspath)
{
	Create(fspath);
}

bool CNativeFileSystem::Create(const std::string& fspath)
{
	if (fspath.empty())
		throw std::runtime_error("Invalid path for native fs.");

	m_fsRoot = std::filesystem::weakly_canonical(std::filesystem::path(fspath));
	return true;
}

void CNativeFileSystem::Copy(const std::string& from, const std::string& to)
{
	auto fromp = std::filesystem::weakly_canonical(m_fsRoot / from);
	auto top = std::filesystem::weakly_canonical(m_fsRoot / to);

	std::filesystem::copy(fromp, top);
}

bool CNativeFileSystem::CreateDirectories(const std::string& path)
{
	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);
	return std::filesystem::create_directories(pathp);
}

bool CNativeFileSystem::CreateDirectory(const std::string& path)
{
	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);
	return std::filesystem::create_directory(pathp);
}

bool CNativeFileSystem::Exists(const std::string& path)
{
	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);
	return std::filesystem::exists(pathp);
}

size_t CNativeFileSystem::FileSize(const std::string& path)
{
	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);
	return std::filesystem::file_size(pathp);
}

bool CNativeFileSystem::IsDirectory(const std::string& path)
{
	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);
	return std::filesystem::is_directory(pathp);
}

bool CNativeFileSystem::IsFile(const std::string& path)
{
	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);
	return std::filesystem::is_regular_file(pathp);
}

bool CNativeFileSystem::Remove(const std::string& path)
{
	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);
	return std::filesystem::remove(pathp);
}

void CNativeFileSystem::Rename(const std::string& original, const std::string& newfile)
{
	auto fromp = std::filesystem::weakly_canonical(m_fsRoot / original);
	auto top = std::filesystem::weakly_canonical(m_fsRoot / newfile);
	std::filesystem::rename(fromp, top);
}

std::unique_ptr<IOutputFileInterface> CNativeFileSystem::CreateFile(const std::string& path)
{
	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);
	return std::make_unique<CNativeOutputFile>(pathp.string());
}

std::unique_ptr<IInputFileInterface> CNativeFileSystem::OpenFile(const std::string& path)
{
	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);

	if (!IsFile(pathp.string()))
		return nullptr;

	return std::make_unique<CNativeInputFile>(pathp.string());
}

std::unique_ptr<IDirectoryIterator> CNativeFileSystem::Walk(const std::string& path, bool recursive)
{
	std::unique_ptr<IDirectoryIterator> iterator{ nullptr };

	auto pathp = std::filesystem::weakly_canonical(m_fsRoot / path);
	if (recursive)
		iterator = std::make_unique<CNativeIterator<std::filesystem::recursive_directory_iterator>>(pathp.string(), m_fsRoot.string());
	else
		iterator = std::make_unique<CNativeIterator<std::filesystem::directory_iterator>>(pathp.string(), m_fsRoot.string());

	return iterator;
}

EFilesystemModeFlags CNativeFileSystem::GetFilesystemMode() const
{
	return static_cast<uint32_t>(EFilesystemModeFlagBits::eRead) | static_cast<uint32_t>(EFilesystemModeFlagBits::eWrite);
}