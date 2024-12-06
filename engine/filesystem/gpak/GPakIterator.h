#pragma once

#include <ie_vfs_exports.h>

#include <engine/interface/filesystem/DirectoryIteratorInterface.h>

struct gpak;
struct filesystem_tree_iterator;
struct filesystem_tree_node;

namespace engine
{
    namespace filesystem
    {
        /**
         * @class GPakIterator
         * @brief This class provides an iterator for GPak archives.
         *
         * GPakIterator is a derived class from IDirectoryIterator and implements its
         * interface for iterating through the files within a gpak archive.
         */
        class INCENERATE_VFS_EXPORT GPakIterator final : public IDirectoryIterator
        {
        public:
            /**
             * @brief Default constructor.
             */
            GPakIterator() = default;

            /**
             * @brief Construct a GPakIterator with a gpak archive, a starting path, and a recursion flag.
             *
             * @param _pak A pointer to the gpak archive.
             * @param start_path A string representing the starting path within the archive.
             * @param recursive A boolean flag indicating if the iterator should recurse into subdirectories.
             */
            GPakIterator(gpak* _pak, std::string_view start_path, bool recursive);

            /**
             * @brief Virtual destructor.
             */
            virtual ~GPakIterator() override;
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
            filesystem_tree_iterator* m_pIterator{ nullptr };
            filesystem_tree_node* m_pCurrentDir{ nullptr };
            std::string m_srCurrentPath{};
            bool m_bIsRecursive{ false };
        };
    }
}