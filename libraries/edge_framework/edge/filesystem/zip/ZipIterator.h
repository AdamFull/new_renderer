#pragma once

#include "../DirectoryIteratorInterface.h"

#include <vector>

struct zip;

namespace edge
{
    namespace filesystem
    {
        /**
         * @class ZipIterator
         * @brief This class provides an iterator for ZIP archives.
         *
         * ZipIterator is a derived class from IDirectoryIterator and implements its
         * interface for iterating through the files within a ZIP archive.
         */
        class ZipIterator final : public IDirectoryIterator
        {
        public:
            /**
             * @brief Default constructor.
             */
            ZipIterator() = default;

            /**
             * @brief Construct a ZipIterator with a zip archive, a starting path, and a recursion flag.
             *
             * @param archive A pointer to the zip archive.
             * @param start_path A string representing the starting path within the archive.
             * @param recursive A boolean flag indicating if the iterator should recurse into subdirectories.
             */
            ZipIterator(zip* archive, std::string_view start_path, bool recursive);

            /**
             * @brief Virtual destructor.
             */
            virtual ~ZipIterator() override = default;

        protected:
            /**
             * @brief Move to the next item in the archive.
             */
            void _OperatorIncrement() override;

            /**
             * @brief Get the current item in the archive.
             *
             * @return A constant reference to the item's string representation.
             */
            const std::string& _OperatorDereferencing() const override;

            /**
             * @brief Compare the directory iterators for inequality.
             *
             * @param other A pointer to the IDirectoryIteratorInterface to compare.
             * @return True if the iterators are not equal, false otherwise.
             */
            bool _OperatorNotEqual(const IDirectoryIteratorInterface*) const override;

        protected:
            /**
             * @brief A pointer to the zip archive.
             */
            zip* archive_{ nullptr };

            /**
             * @brief The index of the current entry in the archive.
             */
            size_t index_{ 0 };

            /**
             * @brief A vector of strings representing the entries in the archive.
             */
            std::vector<std::string> entries_;
        };
    }
}