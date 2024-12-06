#pragma once

#include <ie_vfs_exports.h>

#include <engine/interface/filesystem/VirtualFileSystemInterface.h>

#include <container/hash_map.h>
#include <mutex>

#ifdef CreateDirectory
#undef CreateDirectory
#endif

#ifdef CreateFile
#undef CreateFile
#endif

namespace engine
{
	namespace filesystem
	{
		/**
		 * @class CFilesystemWrapper
		 * @brief A final wrapper class that provides a unified interface for file system operations.
		 *
		 * CFilesystemWrapper allows mounting different file systems with IVFSInterface implementations,
		 * and transparently performing operations on them, such as copy, create, delete, etc.
		 */
		class INCENERATE_VFS_EXPORT VirtualFileSystemManager final : public IVirtualFileSystemInterface
		{
		public:
			/**
			 * @brief Destructor.
			 */
			~VirtualFileSystemManager() override;

			/**
			 * @brief Mounts a filesystem at a specified mount point.
			 * @param mountpoint The mount point as a string.
			 * @param vfsptr A unique_ptr to the filesystem implementation (IVFSInterface).
			 */
			void Mount(std::string_view mountpoint, std::unique_ptr<IVFSInterface>&& vfsptr) override;

			/**
			 * @brief Unmounts a filesystem from a specified mount point.
			 * @param mountpoint The mount point as a string.
			 */
			void Unmount(std::string_view mountpoint) override;

			/**
			 * @brief Copies a file from a source to a destination path or filesystem.
			 * @param from The source file path.
			 * @param to The destination file path.
			 */
			void Copy(std::string_view from, std::string_view to) override;

			/**
			 * @brief Creates directories recursively, if they do not exist.
			 * @param path The directory path to be created.
			 * @return true if the directories were created or already exist, false otherwise.
			 */
			bool CreateDirectories(std::string_view path) override;

			/**
			 * @brief Creates a single directory if it does not exist.
			 * @param path The directory path to be created.
			 * @return true if the directory was created or already exists, false otherwise.
			 */
			bool CreateDirectory(std::string_view path) override;

			/**
			 * @brief Checks whether the specified path exists.
			 * @param path The path to be checked.
			 * @return true if the path exists, false otherwise.
			 */
			bool Exists(std::string_view path) override;

			/**
			 * @brief Retrieves the size of a file at the specified path.
			 * @param path The file path.
			 * @return The size of the file in bytes.
			 */
			size_t FileSize(std::string_view path) override;

			/**
			 * @brief Checks whether the specified path is a directory.
			 * @param path The path to be checked.
			 * @return true if the path is a directory, false otherwise.
			 */
			bool IsDirectory(std::string_view path) override;

			/**
			 * @brief Checks whether the specified path is a file.
			 * @param path The path to be checked.
			 * @return true if the path is a file, false otherwise.
			 */
			bool IsFile(std::string_view path) override;

			/**
			 * @brief Removes a file or an empty directory at the specified path.
			 * @param path The path to be removed.
			 * @return true if the path was successfully removed, false otherwise.
			 */
			bool Remove(std::string_view path) override;

			/**
			 * @brief Renames a file or a directory at the specified original path to a new path.
			 * @param original The original file or directory path.
			 * @param newfile The new file or directory path.
			 */
			void Rename(std::string_view original, std::string_view newfile) override;

			/**
			 * @brief Creates a new file at the specified path.
			 * @param path The file path.
			 * @return A unique_ptr to the created VFSOutFile object.
			 */
			std::unique_ptr<IVFSOutputFileInterface> CreateFile(std::string_view path) override;

			/**
			 * @brief Opens an existing file at the specified path.
			 * @param path The file path.
			 * @return A unique_ptr to the opened VFSInFile object.
			 */
			std::unique_ptr<IVFSInputFileInterface> OpenFile(std::string_view path) override;

			/**
			 * @brief Walks through the directories and files at the specified path.
			 * @param path The path to start walking from.
			 * @param recursive true if the walk should be recursive, false otherwise (default: true).
			 * @return A unique_ptr to the DirectoryIterator object.
			 */
			std::unique_ptr<IDirectoryIteratorInterface> Walk(std::string_view path, bool recursive = true) override;

			bool ReadFile(std::string_view path, char* data, size_t size) override;
			bool ReadFile(std::string_view path, std::string& data) override;
			bool ReadFile(std::string_view path, utl::vector<uint8_t>& data) override;
			bool ReadFile(std::string_view path, utl::vector<uint32_t>& data) override;

			bool WriteFile(std::string_view path, const std::string& data) override;
			bool WriteFile(std::string_view path, const utl::vector<uint8_t>& binary) override;
		private:
			/**
			 * @brief Retrieves the filesystem implementation for the given path.
			 * @param path The file path.
			 * @return A const reference to the unique_ptr of the filesystem implementation (IVFSInterface).
			 */
			const std::unique_ptr<IVFSInterface>& GetFilesystem(std::string_view path);

			/**
			 * @brief Processes the file copy operation between two filesystems.
			 * @param pFSFrom A unique_ptr to the source filesystem implementation (IVFSInterface).
			 * @param pFSTo A unique_ptr to the destination filesystem implementation (IVFSInterface).
			 * @param path The file path as a string.
			 */
			static void ProcessCopyFile(const std::unique_ptr<IVFSInterface>& pFSFrom, const std::unique_ptr<IVFSInterface>& pFSTo, std::string_view path);
		private:
			utl::hash_map<std::string, std::unique_ptr<IVFSInterface>> m_vMountedFS{};
			std::mutex m_SynchronizationMutex;
		};
	}
}