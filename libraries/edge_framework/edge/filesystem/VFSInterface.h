#pragma once

#include "DirectoryIteratorInterface.h"
#include "InputFileInterface.h"
#include "OutputFileInterface.h"

#ifdef CreateFile
#undef CreateFile
#endif

#ifdef CreateDirectory
#undef CreateDirectory
#endif

namespace edge
{
	namespace filesystem
	{
		/**
		 * @enum EFilesystemModeFlagBits
		 * @brief Enumerator for file system mode flag bits.
		 */
		enum class EFilesystemModeFlagBits
		{
			eNone = 1 << 0, ///< No file system mode flag.
			eRead = 1 << 1, ///< Read mode flag.
			eWrite = 1 << 2, ///< Write mode flag.
		};

		/**
		 * @typedef EFilesystemModeFlags
		 * @brief An alias for uint32_t representing a combination of file system mode flag bits.
		 */
		using EFilesystemModeFlags = uint32_t;

		/**
		 * @class IVFSInterface
		 * @brief Interface for virtual file system implementations.
		 *
		 * IVFSInterface provides a common interface for file system operations, such as
		 * create, copy, check existence, etc. It must be implemented by any file system
		 * to be supported by the CFilesystemWrapper.
		 */
		class IVFSInterface
		{
		public:
			/**
			 * @brief Virtual destructor.
			 */
			virtual ~IVFSInterface() = default;

			/**
			 * @brief Initializes the file system.
			 * @param fspath The file system path.
			 * @return true if the file system is initialized successfully, false otherwise.
			 */
			[[nodiscard]] virtual auto Create(std::string_view fspath) -> bool = 0;

			/**
			 * @brief Copies a file or directory from the source to the destination path.
			 * @param src The source path.
			 * @param dst The destination path.
			 */
			virtual void Copy(std::string_view src, std::string_view dst) = 0;

			/**
			 * @brief Creates directories recursively at the specified path, if they do not exist.
			 * @param path The directory path to be created.
			 * @return true if the directories were created or already exist, false otherwise.
			 */
			[[nodiscard]] virtual auto CreateDirectories(std::string_view path) -> bool = 0;

			/**
			 * @brief Creates a single directory at the specified path, if it does not exist.
			 * @param path The directory path to be created.
			 * @return true if the directory was created or already exists, false otherwise.
			 */
			[[nodiscard]] virtual auto CreateDirectory(std::string_view path) -> bool = 0;

			/**
			 * @brief Checks whether the specified path exists.
			 * @param path The path to be checked.
			 * @return true if the path exists, false otherwise.
			 */
			[[nodiscard]] virtual auto Exists(std::string_view path) -> bool = 0;

			/**
			 * @brief Retrieves the size of a file at the specified path.
			 * @param path The file path.
			 * @return The size of the file in bytes.
			 */
			[[nodiscard]] virtual auto FileSize(std::string_view path) -> size_t = 0;

			/**
			 * @brief Checks whether the specified path is a directory.
			 * @param path The path to be checked.
			 * @return true if the path is a directory, false otherwise.
			 */
			[[nodiscard]] virtual auto IsDirectory(std::string_view path) -> bool = 0;

			/**
			 * @brief Checks whether the specified path is a file.
			 * @param path The path to be checked.
			 * @return true if the path is a file, false otherwise.
			 */
			[[nodiscard]] virtual auto IsFile(std::string_view path) -> bool = 0;

			/**
			 * @brief Removes a file or an empty directory at the specified path.
			 * @param path The path to be removed.
			 * @return true if the path was successfully removed, false otherwise.
			 */
			[[nodiscard]] virtual auto Remove(std::string_view path) -> bool = 0;

			/**
			 * @brief Renames a file or a directory at the specified original path to a new path.
			 * @param original The original file or directory path.
			 * @param newfile The new file or directory path.
			 */
			virtual void Rename(std::string_view original, std::string_view newfile) = 0;

			/**
			 * @brief Creates a new file with write mode (wb) at the specified path.
			 * @param path The file path.
			 * @return A unique_ptr to the IOutputFileInterface object representing the created file.
			 */
			[[nodiscard]] virtual auto CreateFile(std::string_view path) -> std::unique_ptr<IOutputFileInterface> = 0;

			/**
			 * @brief Opens an existing file with read mode (rb) at the specified path.
			 * @param path The file path.
			 * @return A unique_ptr to the IInputFileInterface object representing the opened file.
			 */
			[[nodiscard]] virtual auto OpenFile(std::string_view path) -> std::unique_ptr<IInputFileInterface> = 0;

			/**
			 * @brief Creates a recursive directory iterator for the specified path.
			 * @param path The path to start the iteration from.
			 * @param recursive true if the iteration should be recursive, false otherwise.
			 * @return A unique_ptr to the IDirectory
			 */
			[[nodiscard]] virtual auto Walk(std::string_view path, bool recursive) -> std::unique_ptr<IDirectoryIterator> = 0;

			/**
			 * @brief Gets the file system mode flags.
			 * @return A combination of EFilesystemModeFlagBits as EFilesystemModeFlags.
			 */
			[[nodiscard]] virtual auto GetFilesystemMode() const noexcept -> EFilesystemModeFlags = 0;
		};
	}
}