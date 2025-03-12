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
			auto pathp = GetPath(path);
			return new NativeFile(pathp.string(), mode);
		}

		auto NativeFilesystem::FileSize(std::string_view path) -> size_t
		{
			auto pathp = GetPath(path);
			return std::filesystem::file_size(pathp);
		}

		auto NativeFilesystem::CreateDirectories(std::string_view path) -> bool
		{
			auto pathp = GetPath(path);
			return std::filesystem::create_directories(pathp);
		}

		auto NativeFilesystem::CreateDirectory(std::string_view path) -> bool
		{
			auto pathp = GetPath(path);
			return std::filesystem::create_directory(pathp);
		}

		auto NativeFilesystem::Exists(std::string_view path) -> bool
		{
			auto pathp = GetPath(path);
			return std::filesystem::exists(pathp);
		}

		auto NativeFilesystem::IsDirectory(std::string_view path) -> bool
		{
			auto pathp = GetPath(path);
			return std::filesystem::is_directory(pathp);
		}

		auto NativeFilesystem::IsFile(std::string_view path) -> bool
		{
			auto pathp = GetPath(path);
			return std::filesystem::is_regular_file(pathp);
		}

		auto NativeFilesystem::Remove(std::string_view path) -> bool
		{
			auto pathp = GetPath(path);
			return std::filesystem::remove(pathp);
		}

		auto NativeFilesystem::Walk(std::string_view path, bool recursive) -> IDirectoryIterator*
		{
			auto pathp = GetPath(path);
			if (recursive)
				return new NativeDirectoryIterator<std::filesystem::recursive_directory_iterator>(pathp.string(), fs_root.string());

			return new NativeDirectoryIterator<std::filesystem::directory_iterator>(pathp.string(), fs_root.string());
		}

		auto NativeFilesystem::GetPath(std::string_view local_path) -> std::filesystem::path
		{
			return std::filesystem::weakly_canonical(fs_root / local_path);
		}
	}
}