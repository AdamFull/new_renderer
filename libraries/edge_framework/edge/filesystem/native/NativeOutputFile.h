#pragma once

#include "../OutputFileInterface.h"

#include <string>

namespace edge
{
    namespace filesystem
    {
        /**
         * @class NativeOutputFile
         * @brief This class provides an interface to work with output files using native file I/O.
         *
         * NativeOutputFile is a derived class from IOutputFileInterface and provides
         * an implementation for working with output files using native file I/O functions.
         */
        class NativeOutputFile final : public IOutputFileInterface
        {
        public:
            /**
             * @brief Construct a NativeOutputFile and open the file with the given filename.
             *
             * @param filename A string representing the filename of the file to be opened.
             */
            NativeOutputFile(std::string_view filename);

            /**
             * @brief Destructor. Closes the file if it is still open.
             */
            ~NativeOutputFile() override;

            /**
             * @brief Close the file.
             */
            void Close() override;

            /**
             * @brief Check if the file is open.
             *
             * @return True if the file is open, false otherwise.
             */
            bool IsOpen() const override;

            /**
             * @brief Write a character to the file.
             *
             * @param character The character to be written.
             * @return The character written or EOF if an error occurs.
             */
            int32_t Putc(int32_t character) override;

            /**
             * @brief Write data from the buffer to the file.
             *
             * @param buffer A pointer to the buffer containing the data to be written.
             * @param elemSize The size of each element to be written.
             * @param elemCount The number of elements to be written.
             * @return The number of elements successfully written.
             */
            std::size_t Write(const void* buffer, std::size_t elemSize, std::size_t elemCount) override;

            /**
             * @brief Get the current file position indicator.
             *
             * @return The current file position indicator, or -1L if an error occurs.
             */
            long Tell() override;

            /**
             * @brief Set the file position indicator to the specified offset relative to the given origin.
             *
             * @param offset The number of bytes from the given origin.
             * @param origin The position used as a reference for the offset. It can be SEEK_SET, SEEK_CUR, or SEEK_END.
             * @return Zero if successful, or a non-zero value if an error occurs.
             */
            int32_t Seek(long offset, int32_t origin) override;

            /**
             * @brief Flush the output buffer to the file.
             *
             * @return Zero if the buffer is successfully flushed, or EOF if an error occurs.
             */
            int32_t Flush() override;
        private:
            /**
             * @brief A pointer to the FILE structure.
             */
            FILE* m_pFilePtr{ nullptr };
        };
    }
}