#pragma once

#include <ie_vfs_exports.h>

#include <engine/interface/filesystem/DirectoryIteratorInterface.h>

namespace engine
{
    namespace filesystem
    {
        /**
         * @class DirectoryIterator
         * @brief This class provides a directory iterator implementation.
         */
        class INCENERATE_VFS_EXPORT DirectoryIterator : public IDirectoryIteratorInterface
        {
        public:
            /**
             * @brief Default constructor.
             */
            DirectoryIterator() = default;

            /**
             * @brief Construct a DirectoryIterator from a unique_ptr to an IDirectoryIterator and a mountpoint string.
             *
             * @param it A unique_ptr to an IDirectoryIterator.
             * @param mountpoint A string representing the mountpoint.
             */
            DirectoryIterator(std::unique_ptr<IDirectoryIterator>&& it, std::string_view mountpoint);

            /**
             * @brief Destructor.
             */
            ~DirectoryIterator() override = default;

            /**
             * @brief Get the iterator pointing to the beginning of the directory.
             *
             * @return A reference to the iterator pointing to the beginning of the directory.
             */
            DirectoryIterator& Begin() override;

            /**
             * @brief Get the iterator pointing to the end of the directory.
             *
             * @return A reference to the iterator pointing to the end of the directory.
             */
            DirectoryIterator& End() override;

            /**
             * @brief Move to the next item in the directory.
             *
             * @return A reference to the updated iterator.
             */
            DirectoryIterator& operator++() override;

            /**
             * @brief Get the current item in the directory.
             *
             * @return A constant reference to the item's string representation.
             */
            const std::string& operator*() override;

            /**
             * @brief Compare the directory iterators for inequality.
             *
             * @param other A reference to the DirectoryIterator to compare.
             * @return True if the iterators are not equal, false otherwise.
             */
            bool operator!=(const IDirectoryIteratorInterface& other) const override;
        private:
            std::string m_srMountPoint{};
            std::string m_srCurrent{};
        };
    }
}