#include "VFSOutputFile.h"

using namespace engine::filesystem;

CVFSOutFile::CVFSOutFile(std::unique_ptr<IOutputFileInterface>&& pFile) :
	m_pFileInterface(std::move(pFile))
{
	if (!m_pFileInterface)
		throw std::runtime_error("Failed to create file");
}

CVFSOutFile::~CVFSOutFile()
{
	if (m_pFileInterface)
		m_pFileInterface->Close();
}

bool CVFSOutFile::IsOpen()
{
	if (m_pFileInterface)
		return m_pFileInterface->IsOpen();

	return false;
}

CVFSOutFile& CVFSOutFile::Put(char ch)
{
	if (m_pFileInterface)
		m_pFileInterface->Putc(ch);

	return *this;
}

CVFSOutFile& CVFSOutFile::Write(const std::string& str)
{
	if (m_pFileInterface)
		m_pFileInterface->Write(str.c_str(), sizeof(char), str.size());

	return *this;
}

CVFSOutFile& CVFSOutFile::Write(const utl::vector<uint8_t>& data)
{
	if (m_pFileInterface)
		m_pFileInterface->Write(data.data(), sizeof(uint8_t), data.size());

	return *this;
}

CVFSOutFile& CVFSOutFile::Write(const char* buffer, std::size_t count)
{
	if (m_pFileInterface)
		m_pFileInterface->Write(buffer, sizeof(char), count);

	return *this;
}

std::streampos CVFSOutFile::Tellp()
{
	return m_pFileInterface ? m_pFileInterface->Tell() : -1;
}

CVFSOutFile& CVFSOutFile::Seekp(std::streampos pos)
{
	if (m_pFileInterface)
		m_pFileInterface->Seek(pos, SEEK_SET);

	return *this;
}

CVFSOutFile& CVFSOutFile::Flush()
{
	if (m_pFileInterface)
		m_pFileInterface->Flush();

	return *this;
}