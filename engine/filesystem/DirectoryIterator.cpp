#include "DirectoryIterator.h"

using namespace engine::filesystem;

CDirectoryIterator::CDirectoryIterator(std::unique_ptr<IDirectoryIterator>&& it, const std::string& mountpoint) :
    m_srMountPoint(mountpoint)
{
    it_ = std::move(it);
}

CDirectoryIterator& CDirectoryIterator::Begin()
{ 
    return *this; 
}

CDirectoryIterator& CDirectoryIterator::End()
{
    static CDirectoryIterator end_{};
    return end_;
}

CDirectoryIterator& CDirectoryIterator::operator++()
{
    it_->_OperatorIncrement();
    return *this;
}

const std::string& CDirectoryIterator::operator*()
{
    m_srCurrent = m_srMountPoint + "/" + it_->_OperatorDereferencing();
    return m_srCurrent;
}

bool CDirectoryIterator::operator!=(const IDirectoryIteratorInterface& other) const
{
    return it_->_OperatorNotEqual(&other); 
}