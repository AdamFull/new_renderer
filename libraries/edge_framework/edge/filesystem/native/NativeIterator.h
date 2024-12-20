#pragma once

#include "ie_vfs_exports.h"

#include "../../util/fs_helper.h"

#include "../DirectoryIteratorInterface.h"

#include <array>
#include <string>
#include <codecvt>

namespace edge
{
    namespace filesystem
    {
        /**
         * @class NativeIterator
         * @brief A template class providing an interface for native directory iteration.
         
         * NativeIterator is a derived class from IDirectoryIterator that provides
         * a platform-specific implementation for iterating through directories using the
         * native filesystem functions. The class is templated to allow for different types
         * of directory iterators.
         *
         * @tparam _Ty The type of native directory iterator to be used.
         */
        template<class _Ty>
        class INCENERATE_VFS_EXPORT NativeIterator final : public IDirectoryIterator
        {
        public:
            /**
             * @brief Default constructor.
             */
            NativeIterator() = default;

            /**
             * @brief Construct a NativeIterator for the specified path and root.
             *
             * @param path A string representing the path to the directory to be iterated.
             * @param root A string representing the root directory for relative paths.
             */
            NativeIterator(std::string_view path, std::string_view root) : it_(_Ty(path))
            {
                root_ = root;

                std::array<char, 1024ull> buffer{};
                path_ = fsh::to_posix_path(std::filesystem::relative(it_->path(), root_).string(), buffer);
            }
        protected:
            /**
             * @brief Increment the iterator to the next entry in the directory.
             */
            void _OperatorIncrement() override
            {
                ++it_;
                if (it_ != _Ty())
                {
                    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
                    auto decoded = convert.to_bytes(std::filesystem::relative(it_->path(), root_).u16string());

                    std::array<char, 1024ull> buffer{};
                    path_ = fsh::to_posix_path(decoded, buffer);
                }
            }

            /**
             * @brief Dereference the iterator to get the current entry's relative path.
             *
             * @return A reference to a string representing the current entry's relative path.
             */
            const std::string& _OperatorDereferencing() const override { return path_; }

            /**
             * @brief Compare this iterator with another IDirectoryIteratorInterface.
             *
             * @param other A pointer to the IDirectoryIteratorInterface to be compared with.
             * @return True if the iterators are not equal, false otherwise.
             */
            bool _OperatorNotEqual(const IDirectoryIteratorInterface* other) const override
            {
                if (!other->it_)
                    return it_ != _Ty();

                const auto* other_it = dynamic_cast<const NativeIterator<_Ty>*>(other->it_.get());
                return it_ != other_it->it_;
            }
        private:
            /**
             * @brief The native directory iterator.
             */
            _Ty it_;

            /**
             * @brief The relative path of the current file or directory.
             */
            std::string path_;

            /**
             * @brief The root directory for relative paths.
             */
            std::string root_;
        };
    }
}