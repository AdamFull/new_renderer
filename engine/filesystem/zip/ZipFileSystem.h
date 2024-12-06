#pragma once

#include <ie_vfs_exports.h>

#include <engine/interface/filesystem/VFSInterface.h>

struct zip;

namespace engine
{
	namespace filesystem
	{
		/**
		 * @class ZipFileSystem
		 * @brief This class provides an interface for virtual file systems within a ZIP archive.
		 *
		 * ZipFileSystem is a derived class from IVFSInterface and implements its
		 * interface for working with files and directories within a ZIP archive.
		 */
		class INCENERATE_VFS_EXPORT ZipFileSystem final : public IVFSInterface
		{
		public:
			/**
			 * @brief Construct a ZipFileSystem with a file system path and a password.
			 *
			 * @param fspath A string representing the file system path of the ZIP archive.
			 * @param password A string representing the password for encrypted files.
			 */
			ZipFileSystem(const char* fspath, const char* password);

			/**
			 * @brief Destructor.
			 */
			virtual ~ZipFileSystem() override;

			/**
			 * @brief Create a new ZIP archive at the specified file system path.
			 *
			 * @param fspath A string representing the file system path of the new ZIP archive.
			 * @return True if the archive is successfully created, false otherwise.
			 */
			bool Create(const char* fspath) override;

			/**
			 * @brief This function is not supported by the ZipFileSystem.
			 */
			void Copy(const char* from, const char* to) override {}

			/**
			 * @brief This function is not supported by the ZipFileSystem.
			 */
			bool CreateDirectories(const char* path) override { return false; }

			/**
			 * @brief This function is not supported by the ZipFileSystem.
			 */
			bool CreateDirectory(const char* path) override { return false; }

			/**
		 * @brief Check if an entry exists at the specified path within the ZIP archive.
		 *
		 * @param path A string representing the path of the entry.
		 * @return True if the entry exists, false otherwise.
		 */
			bool Exists(const char* path) override;

			/**
			 * @brief Get the size of a file at the specified path within the ZIP archive.
			 *
			 * @param path A string representing the path of the file.
			 * @return The size of the file in bytes.
			 */
			size_t FileSize(const char* path) override;

			/**
			 * @brief Check if a directory exists at the specified path within the ZIP archive.
			 *
			 * @param path A string representing the path of the directory.
			 * @return True if the directory exists, false otherwise.
			 */
			bool IsDirectory(const char* path) override;

			/**
			 * @brief Check if a file exists at the specified path within the ZIP archive.
			 *
			 * @param path A string representing the path of the file.
			 * @return True if the file exists, false otherwise.
			 */
			bool IsFile(const char* path) override;

			/**
			 * @brief This function is not supported by the ZipFileSystem.
			 */
			bool Remove(const char* path) override { return false; }

			/**
			 * @brief This function is not supported by the ZipFileSystem.
			 */
			void Rename(const char* original, const char* newfile) override { }

			/**
			 * @brief This function is not supported by the ZipFileSystem.
			 */
			std::unique_ptr<IOutputFileInterface> CreateFile(const char* path) override { return nullptr; }

			/**
			 * @brief Open a file at the specified path within the ZIP archive.
			 *
			 * @param path A string representing the path of the file.
			 * @return A unique_ptr to an IInputFileInterface if the file is successfully opened, nullptr otherwise.
			 */
			std::unique_ptr<IInputFileInterface> OpenFile(const char* path) override;

			/**
			 * @brief Create a directory iterator for the specified path within the ZIP archive.
			 *
			 * This method creates an IDirectoryIterator to traverse the contents of a directory
			 * within the ZIP archive. If recursive is set to true, the iterator will also
			 * traverse subdirectories.
			 *
			 * @param path A string representing the path of the directory.
			 * @param recursive A boolean flag indicating whether to traverse subdirectories.
			 * @return A unique_ptr to an IDirectoryIterator for the specified path.
			 */
			std::unique_ptr<IDirectoryIterator> Walk(const char* path, bool recursive) override;

		protected:
			/**
			 * @brief Get the file system mode flags for the ZIP file system.
			 *
			 * @return The file system mode flags.
			 */
			EFilesystemModeFlags GetFilesystemMode() const override;

		private:
			/**
			 * @brief A pointer to the zip structure.
			 */
			zip* m_pArchive{ nullptr };

			/**
			 * @brief The password for encrypted files.
			 */
			std::string m_srPassword{};
		};
	}
}