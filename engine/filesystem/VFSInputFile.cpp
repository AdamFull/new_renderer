#include "VFSInputFile.h"

using namespace engine::filesystem;

CVFSInFile::CVFSInFile(std::unique_ptr<IInputFileInterface>&& pFile) :
	m_pFileInterface(std::move(pFile))
{
	if (!m_pFileInterface)
		throw std::runtime_error("Failed to open file");
}

CVFSInFile::~CVFSInFile()
{
	if (m_pFileInterface)
		m_pFileInterface->Close();
}

bool CVFSInFile::IsOpen()
{
	if (m_pFileInterface)
		return m_pFileInterface->IsOpen();

	return false;
}

int CVFSInFile::Get()
{
	m_gcount = 0ull;

	int ch = m_pFileInterface ? m_pFileInterface->Getc() : EOF;
	if (ch != EOF) 
		m_gcount = 1ull;

	return ch;
}

std::string CVFSInFile::GetLine()
{
	m_gcount = 0ull;

	if (!m_pFileInterface)
		return {};

	std::string line;

	int ch;
	while ((ch = m_pFileInterface->Getc()) != EOF && ch != '\n')
	{
		line.push_back(static_cast<char>(ch));
		++m_gcount;
	}

	return line;
}

std::size_t CVFSInFile::Gcount() const
{
	return m_gcount;
}

CVFSInFile& CVFSInFile::Ignore(std::size_t count, int delimiter)
{
	if (m_pFileInterface)
	{
		for (std::size_t i = 0; i < count; ++i) 
		{
			int ch = m_pFileInterface->Getc();
			if (ch == EOF || ch == delimiter)
				break;
		}
	}

	return *this;
}

int CVFSInFile::Peek() const
{
	return m_pFileInterface ? m_pFileInterface->Getc() : EOF;
}

CVFSInFile& CVFSInFile::Read(char* buffer, std::size_t count)
{
	m_gcount = 0ull;

	if (m_pFileInterface)
		m_gcount = m_pFileInterface->Read(buffer, sizeof(char), count);

	return *this;
}

CVFSInFile& CVFSInFile::Read(std::string& buffer)
{
	Seekg(0, SEEK_END);
	auto filesize = Tellg();
	Seekg(0);

	buffer.resize(filesize);
	Read(buffer.data(), buffer.size());

	return *this;
}

CVFSInFile& CVFSInFile::Read(utl::vector<uint8_t>& buffer)
{
	Seekg(0, SEEK_END);
	auto filesize = Tellg();
	Seekg(0);

	buffer.resize(filesize);
	if (m_pFileInterface)
		m_gcount = m_pFileInterface->Read(buffer.data(), sizeof(uint8_t), filesize);

	return *this;
}

CVFSInFile& CVFSInFile::Unget()
{
	if (m_pFileInterface)
		m_pFileInterface->Ungetc(Peek());

	return *this;
}

std::streampos CVFSInFile::Tellg()
{
	return m_pFileInterface ? m_pFileInterface->Tell() : -1;
}

CVFSInFile& CVFSInFile::Seekg(std::streampos pos)
{
	Seekg(pos, SEEK_SET);

	return *this;
}

CVFSInFile& CVFSInFile::Seekg(std::streampos pos, int32_t origin)
{
	if (m_pFileInterface)
		m_pFileInterface->Seek(pos, origin);

	return *this;
}

bool CVFSInFile::Eof() const
{
	if (m_pFileInterface)
		return m_pFileInterface->Eof();

	return true;
}