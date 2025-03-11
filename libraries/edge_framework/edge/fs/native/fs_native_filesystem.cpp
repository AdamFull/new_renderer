#include "fs_native_filesystem.h"

#include "fs_native_directory_iterator.h"
#include "fs_native_file.h"

namespace edge
{
	namespace fs
	{
		NativeFilesystem::NativeFilesystem(std::string_view root)
		{
			fs_root = std::filesystem::weakly_canonical(std::filesystem::path(root));
		}

		NativeFilesystem::~NativeFilesystem()
		{

		}

		auto NativeFilesystem::OpenFile(std::string_view path, FileModeFlags mode) -> IFile*
		{
			auto pathp = std::filesystem::weakly_canonical(fs_root / path);
			return new NativeFile(pathp.string(), mode);
		}

		auto NativeFilesystem::CreateDirectories(std::string_view path) -> bool
		{
			auto pathp = std::filesystem::weakly_canonical(fs_root / path);
			return std::filesystem::create_directories(pathp);
		}

		auto NativeFilesystem::CreateDirectory(std::string_view path) -> bool
		{
			auto pathp = std::filesystem::weakly_canonical(fs_root / path);
			return std::filesystem::create_directory(pathp);
		}

		auto NativeFilesystem::Exists(std::string_view path) -> bool
		{
			auto pathp = std::filesystem::weakly_canonical(fs_root / path);
			return std::filesystem::exists(pathp);
		}

		auto NativeFilesystem::IsDirectory(std::string_view path) -> bool
		{
			auto pathp = std::filesystem::weakly_canonical(fs_root / path);
			return std::filesystem::is_directory(pathp);
		}

		auto NativeFilesystem::IsFile(std::string_view path) -> bool
		{
			auto pathp = std::filesystem::weakly_canonical(fs_root / path);
			return std::filesystem::is_regular_file(pathp);
		}

		auto NativeFilesystem::Remove(std::string_view path) -> bool
		{
			auto pathp = std::filesystem::weakly_canonical(fs_root / path);
			return std::filesystem::remove(pathp);
		}

		auto NativeFilesystem::Walk(std::string_view path, bool recursive) -> IDirectoryIterator*
		{
			auto pathp = std::filesystem::weakly_canonical(fs_root / path);
			if (recursive)
				return new NativeDirectoryIterator<std::filesystem::recursive_directory_iterator>(pathp.string(), fs_root.string());

			return new NativeDirectoryIterator<std::filesystem::directory_iterator>(pathp.string(), fs_root.string());
		}
	}
}