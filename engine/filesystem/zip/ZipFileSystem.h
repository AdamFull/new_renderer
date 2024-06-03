#pragma once

#include <ie_vfs_exports.h>

#include <engine/interface/filesystem/VFSInterface.h>

struct zip;

namespace engine
{
	namespace filesystem
	{
		/**
		 * @class CZipFileSystem
		 * @brief This class provides an interface for virtual file systems within a ZIP archive.
		 *
		 * CZipFileSystem is a derived class from IVFSInterface and implements its
		 * interface for working with files and directories within a ZIP archive.
		 */
		class INCENERATE_VFS_EXPORT CZipFileSystem final : public IVFSInterface
		{
		public:
			/**
			 * @brief Construct a CZipFileSystem with a file system path and a password.
			 *
			 * @param fspath A string representing the file system path of the ZIP archive.
			 * @param password A string representing the password for encrypted files.
			 */
			CZipFileSystem(const std::string& fspath, const std::string& password);

			/**
			 * @brief Destructor.
			 */
			virtual ~CZipFileSystem() override;

			/**
			 * @brief Create a new ZIP archive at the specified file system path.
			 *
			 * @param fspath A string representing the file system path of the new ZIP archive.
			 * @return True if the archive is successfully created, false otherwise.
			 */
			bool Create(const std::string& fspath) override;

			/**
			 * @brief This function is not supported by the CZipFileSystem.
			 */
			void Copy(const std::string& from, const std::string& to) override {}

			/**
			 * @brief This function is not supported by the CZipFileSystem.
			 */
			bool CreateDirectories(const std::string& path) override { return false; }

			/**
			 * @brief This function is not supported by the CZipFileSystem.
			 */
			bool CreateDirectory(const std::string& path) override { return false; }

			/**
		 * @brief Check if an entry exists at the specified path within the ZIP archive.
		 *
		 * @param path A string representing the path of the entry.
		 * @return True if the entry exists, false otherwise.
		 */
			bool Exists(const std::string& path) override;

			/**
			 * @brief Get the size of a file at the specified path within the ZIP archive.
			 *
			 * @param path A string representing the path of the file.
			 * @return The size of the file in bytes.
			 */
			size_t FileSize(const std::string& path) override;

			/**
			 * @brief Check if a directory exists at the specified path within the ZIP archive.
			 *
			 * @param path A string representing the path of the directory.
			 * @return True if the directory exists, false otherwise.
			 */
			bool IsDirectory(const std::string& path) override;

			/**
			 * @brief Check if a file exists at the specified path within the ZIP archive.
			 *
			 * @param path A string representing the path of the file.
			 * @return True if the file exists, false otherwise.
			 */
			bool IsFile(const std::string& path) override;

			/**
			 * @brief This function is not supported by the CZipFileSystem.
			 */
			bool Remove(const std::string& path) override { return false; }

			/**
			 * @brief This function is not supported by the CZipFileSystem.
			 */
			void Rename(const std::string& original, const std::string& newfile) override { }

			/**
			 * @brief This function is not supported by the CZipFileSystem.
			 */
			std::unique_ptr<IOutputFileInterface> CreateFile(const std::string& path) override { return nullptr; }

			/**
			 * @brief Open a file at the specified path within the ZIP archive.
			 *
			 * @param path A string representing the path of the file.
			 * @return A unique_ptr to an IInputFileInterface if the file is successfully opened, nullptr otherwise.
			 */
			std::unique_ptr<IInputFileInterface> OpenFile(const std::string& path) override;

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
			std::unique_ptr<IDirectoryIterator> Walk(const std::string& path, bool recursive) override;

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