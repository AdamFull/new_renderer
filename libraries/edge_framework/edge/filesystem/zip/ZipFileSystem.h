#pragma once

#include "../VFSInterface.h"

struct zip;

namespace edge
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
		class ZipFileSystem final : public IVFSInterface
		{
		public:
			/**
			 * @brief Construct a ZipFileSystem with a file system path and a password.
			 *
			 * @param fspath A string representing the file system path of the ZIP archive.
			 * @param password A string representing the password for encrypted files.
			 */
			ZipFileSystem(std::string_view fspath, std::string_view password);

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
			[[nodiscard]] auto Create(std::string_view fspath) -> bool override;

			/**
			 * @brief This function is not supported by the ZipFileSystem.
			 */
			void Copy(std::string_view from, std::string_view to) override {}

			/**
			 * @brief This function is not supported by the ZipFileSystem.
			 */
			[[nodiscard]] auto CreateDirectories(std::string_view path) -> bool override { return false; }

			/**
			 * @brief This function is not supported by the ZipFileSystem.
			 */
			[[nodiscard]] auto CreateDirectory(std::string_view path) -> bool  override { return false; }

			/**
		 * @brief Check if an entry exists at the specified path within the ZIP archive.
		 *
		 * @param path A string representing the path of the entry.
		 * @return True if the entry exists, false otherwise.
		 */
			[[nodiscard]] auto Exists(std::string_view path) -> bool override;

			/**
			 * @brief Get the size of a file at the specified path within the ZIP archive.
			 *
			 * @param path A string representing the path of the file.
			 * @return The size of the file in bytes.
			 */
			[[nodiscard]] auto FileSize(std::string_view path) -> size_t override;

			/**
			 * @brief Check if a directory exists at the specified path within the ZIP archive.
			 *
			 * @param path A string representing the path of the directory.
			 * @return True if the directory exists, false otherwise.
			 */
			[[nodiscard]] auto IsDirectory(std::string_view path) -> bool override;

			/**
			 * @brief Check if a file exists at the specified path within the ZIP archive.
			 *
			 * @param path A string representing the path of the file.
			 * @return True if the file exists, false otherwise.
			 */
			[[nodiscard]] auto IsFile(std::string_view path) -> bool override;

			/**
			 * @brief This function is not supported by the ZipFileSystem.
			 */
			[[nodiscard]] auto Remove(std::string_view path) override { return false; }

			/**
			 * @brief This function is not supported by the ZipFileSystem.
			 */
			void Rename(std::string_view original, std::string_view newfile) override { }

			/**
			 * @brief This function is not supported by the ZipFileSystem.
			 */
			[[nodiscard]] auto CreateFile(std::string_view path) -> std::unique_ptr<IOutputFileInterface> override { return nullptr; }

			/**
			 * @brief Open a file at the specified path within the ZIP archive.
			 *
			 * @param path A string representing the path of the file.
			 * @return A unique_ptr to an IInputFileInterface if the file is successfully opened, nullptr otherwise.
			 */
			[[nodiscard]] auto OpenFile(std::string_view path) -> std::unique_ptr<IInputFileInterface> override;

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
			[[nodiscard]] auto Walk(std::string_view path, bool recursive) -> std::unique_ptr<IDirectoryIterator> override;

		protected:
			/**
			 * @brief Get the file system mode flags for the ZIP file system.
			 *
			 * @return The file system mode flags.
			 */
			[[nodiscard]] auto GetFilesystemMode() const noexcept -> EFilesystemModeFlags override
			{
				return static_cast<uint32_t>(EFilesystemModeFlagBits::eRead);
			}

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