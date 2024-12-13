#include "DirectoryIterator.h"

namespace edge
{
    namespace filesystem
    {
        DirectoryIterator::DirectoryIterator(std::unique_ptr<IDirectoryIterator>&& it, std::string_view mountpoint) :
            m_srMountPoint(mountpoint)
        {
            it_ = std::move(it);
        }

        DirectoryIterator& DirectoryIterator::Begin()
        {
            return *this;
        }

        DirectoryIterator& DirectoryIterator::End()
        {
            static DirectoryIterator end_{};
            return end_;
        }

        DirectoryIterator& DirectoryIterator::operator++()
        {
            it_->_OperatorIncrement();
            return *this;
        }

        const std::string& DirectoryIterator::operator*()
        {
            m_srCurrent = m_srMountPoint + "/" + it_->_OperatorDereferencing();
            return m_srCurrent;
        }

        bool DirectoryIterator::operator!=(const IDirectoryIteratorInterface& other) const
        {
            return it_->_OperatorNotEqual(&other);
        }
    }
}