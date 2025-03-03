#pragma once

#include "../VFSInterface.h"

#include <filesystem>

#ifdef CreateDirectory
#undef CreateDirectory
#endif

#ifdef CreateFile
#undef CreateFile
#endif

namespace edge
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
        class NativeFileSystem final : public IVFSInterface
        {
        public:
            /**
             * @brief Construct a NativeFileSystem for the specified file system path.
             *
             * @param fspath A string representing the file system path to be used.
             */
            NativeFileSystem(std::string_view fspath);

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
            [[nodiscard]] auto Create(std::string_view fspath) -> bool override;

            /**
             * @brief Copy a file from one path to another.
             *
             * @param from A string representing the source file path.
             * @param to A string representing the destination file path.
             */
            void Copy(std::string_view from, std::string_view to) override;

            /**
             * @brief Create a directory hierarchy at the specified path.
             *
             * @param path A string representing the directory hierarchy to create.
             * @return True on success, false on failure.
             */
            [[nodiscard]] auto CreateDirectories(std::string_view path) -> bool override;

            /**
             * @brief Create a directory at the specified path.
             *
             * @param path A string representing the directory to create.
             * @return True on success, false on failure.
             */
            [[nodiscard]] auto CreateDirectory(std::string_view path) -> bool override;

            /**
             * @brief Check if a file or directory exists at the specified path.
             *
             * @param path A string representing the path to check for existence.
             * @return True if the path exists, false otherwise.
             */
            [[nodiscard]] auto Exists(std::string_view path) -> bool override;

            /**
             * @brief Get the size of a file at the specified path.
             *
             * @param path A string representing the path to the file.
             * @return The size of the file in bytes.
             */
            [[nodiscard]] auto FileSize(std::string_view path) -> size_t override;

            /**
             * @brief Check if a path points to a directory.
             *
             * @param path A string representing the path to check.
             * @return True if the path points to a directory, false otherwise.
             */
            [[nodiscard]] auto IsDirectory(std::string_view path) -> bool override;

            /**
             * @brief Check if a path points to a file.
             *
             * @param path A string representing the path to check.
             * @return True if the path points to a file, false otherwise.
             */
            [[nodiscard]] auto IsFile(std::string_view path) -> bool override;

            /**
             * @brief Remove a file or directory at the specified path.
             *
             * @param path A string representing the path to remove.
             * @return True on success, false on failure.
             */
            [[nodiscard]] auto Remove(std::string_view path) -> bool override;

            /**
             * @brief Rename a file or directory.
             *
             * @brief Rename a file or directory.
             * @param original A string representing the original path of the file or directory.
             * @param newfile A string representing the new path for the file or directory.
             */
            void Rename(std::string_view original, std::string_view newfile) override;

            /**
             * @brief Create a new file at the specified path.
             *
             * @param path A string representing the path to create the new file.
             * @return A unique_ptr to an IOutputFileInterface object on success, nullptr on failure.
             */
            [[nodiscard]] auto CreateFile(std::string_view path) -> std::unique_ptr<IOutputFileInterface> override;

            /**
             * @brief Open a file at the specified path for reading.
             *
             * @param path A string representing the path to the file to open.
             * @return A unique_ptr to an IInputFileInterface object on success, nullptr on failure.
            */
            [[nodiscard]] auto OpenFile(std::string_view path) -> std::unique_ptr<IInputFileInterface> override;

            /**
             * @brief Create a directory iterator for walking the file system.
             *
             * @param path A string representing the starting path for the iterator.
             * @param recursive A boolean flag indicating whether the iterator should recurse into subdirectories.
             * @return A unique_ptr to an IDirectoryIterator object on success, nullptr on failure.
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
                return static_cast<uint32_t>(EFilesystemModeFlagBits::eRead) | static_cast<uint32_t>(EFilesystemModeFlagBits::eWrite);
            }

        private:
            std::filesystem::path m_fsRoot{};
        };
    }
}