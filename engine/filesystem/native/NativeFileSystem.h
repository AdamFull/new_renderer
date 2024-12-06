#pragma once

#include <ie_vfs_exports.h>

#include <engine/interface/filesystem/VFSInterface.h>

#include <filesystem>

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
         * @class NativeFileSystem
         * @brief A class for handling native file system operations.
         *
         * NativeFileSystem is a derived class from IVFSInterface that provides
         * a platform-specific implementation for native file system operations.
         */
        class INCENERATE_VFS_EXPORT NativeFileSystem final : public IVFSInterface
        {
        public:
            /**
             * @brief Construct a NativeFileSystem for the specified file system path.
             *
             * @param fspath A string representing the file system path to be used.
             */
            NativeFileSystem(const char* fspath);

            /**
             * @brief Destructor.
             */
            virtual ~NativeFileSystem() override = default;

            /**
             * @brief Create a new file or directory at the specified path.
             *
             * @param fspath A string representing the path to create the new file or directory.
             * @return True on success, false on failure.
             */
            bool Create(const char* fspath) override;

            /**
             * @brief Copy a file from one path to another.
             *
             * @param from A string representing the source file path.
             * @param to A string representing the destination file path.
             */
            void Copy(const char* from, const char* to) override;

            /**
             * @brief Create a directory hierarchy at the specified path.
             *
             * @param path A string representing the directory hierarchy to create.
             * @return True on success, false on failure.
             */
            bool CreateDirectories(const char* path) override;

            /**
             * @brief Create a directory at the specified path.
             *
             * @param path A string representing the directory to create.
             * @return True on success, false on failure.
             */
            bool CreateDirectory(const char* path) override;

            /**
             * @brief Check if a file or directory exists at the specified path.
             *
             * @param path A string representing the path to check for existence.
             * @return True if the path exists, false otherwise.
             */
            bool Exists(const char* path) override;

            /**
             * @brief Get the size of a file at the specified path.
             *
             * @param path A string representing the path to the file.
             * @return The size of the file in bytes.
             */
            size_t FileSize(const char* path) override;

            /**
             * @brief Check if a path points to a directory.
             *
             * @param path A string representing the path to check.
             * @return True if the path points to a directory, false otherwise.
             */
            bool IsDirectory(const char* path) override;

            /**
             * @brief Check if a path points to a file.
             *
             * @param path A string representing the path to check.
             * @return True if the path points to a file, false otherwise.
             */
            bool IsFile(const char* path) override;

            /**
             * @brief Remove a file or directory at the specified path.
             *
             * @param path A string representing the path to remove.
             * @return True on success, false on failure.
             */
            bool Remove(const char* path) override;

            /**
             * @brief Rename a file or directory.
             *
             * @brief Rename a file or directory.
             * @param original A string representing the original path of the file or directory.
             * @param newfile A string representing the new path for the file or directory.
             */
            void Rename(const char* original, const char* newfile) override;

            /**
             * @brief Create a new file at the specified path.
             *
             * @param path A string representing the path to create the new file.
             * @return A unique_ptr to an IOutputFileInterface object on success, nullptr on failure.
             */
            std::unique_ptr<IOutputFileInterface> CreateFile(const char* path) override;

            /**
             * @brief Open a file at the specified path for reading.
             *
             * @param path A string representing the path to the file to open.
             * @return A unique_ptr to an IInputFileInterface object on success, nullptr on failure.
            */
            std::unique_ptr<IInputFileInterface> OpenFile(const char* path) override;

            /**
             * @brief Create a directory iterator for walking the file system.
             *
             * @param path A string representing the starting path for the iterator.
             * @param recursive A boolean flag indicating whether the iterator should recurse into subdirectories.
             * @return A unique_ptr to an IDirectoryIterator object on success, nullptr on failure.
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
            std::filesystem::path m_fsRoot{};
        };
    }
}