#include "fs_native_filesystem.h"

#include "fs_native_directory_iterator.h"
#include "fs_native_file.h"

namespace edge::fs
{
	NativeFilesystem::NativeFilesystem(std::string_view root) {
		fs_root = std::filesystem::weakly_canonical(std::filesystem::path(root));
	}

	NativeFilesystem::~NativeFilesystem() {

	}

	auto NativeFilesystem::open_file(std::string_view path, FileModeFlags mode) -> IFile* {
		auto pathp = get_path(path);
		return new NativeFile(pathp.string(), mode);
	}

	auto NativeFilesystem::file_size(std::string_view path) -> size_t {
		auto pathp = get_path(path);
		return std::filesystem::file_size(pathp);
	}

	auto NativeFilesystem::create_directories(std::string_view path) -> bool {
		auto pathp = get_path(path);
		return std::filesystem::create_directories(pathp);
	}

	auto NativeFilesystem::create_directory(std::string_view path) -> bool {
		auto pathp = get_path(path);
		return std::filesystem::create_directory(pathp);
	}

	auto NativeFilesystem::exists(std::string_view path) -> bool {
		auto pathp = get_path(path);
		return std::filesystem::exists(pathp);
	}

	auto NativeFilesystem::is_directory(std::string_view path) -> bool {
		auto pathp = get_path(path);
		return std::filesystem::is_directory(pathp);
	}

	auto NativeFilesystem::is_file(std::string_view path) -> bool {
		auto pathp = get_path(path);
		return std::filesystem::is_regular_file(pathp);
	}

	auto NativeFilesystem::remove(std::string_view path) -> bool {
		auto pathp = get_path(path);
		return std::filesystem::remove(pathp);
	}

	auto NativeFilesystem::walk(std::string_view path, bool recursive) -> IDirectoryIterator* {
		auto pathp = get_path(path);
		if (recursive)
			return new NativeDirectoryIterator<std::filesystem::recursive_directory_iterator>(pathp.string(), fs_root.string());

		return new NativeDirectoryIterator<std::filesystem::directory_iterator>(pathp.string(), fs_root.string());
	}

	auto NativeFilesystem::get_path(std::string_view local_path) -> std::filesystem::path {
		return std::filesystem::weakly_canonical(fs_root / local_path);
	}
}