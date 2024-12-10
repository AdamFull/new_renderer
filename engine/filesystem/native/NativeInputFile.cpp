#include "NativeInputFile.h"

#include <stdexcept>

using namespace engine::filesystem;

CNativeInputFile::CNativeInputFile(std::string_view filename)
{
	m_pFilePtr = std::fopen(filename.data(), "rb");

	if (!m_pFilePtr)
		throw std::runtime_error("Failed to open native file.");
}

CNativeInputFile::~CNativeInputFile()
{
	Close();
}

void CNativeInputFile::Close()
{
	if (m_pFilePtr)
	{
		fclose(m_pFilePtr);
		m_pFilePtr = nullptr;
	}
}

bool CNativeInputFile::IsOpen() const
{
	return m_pFilePtr;
}

int32_t CNativeInputFile::Getc()
{
	return m_pFilePtr ? std::fgetc(m_pFilePtr) : -1;
}

std::size_t CNativeInputFile::Read(void* buffer, std::size_t elemSize, std::size_t elemCount)
{
	return m_pFilePtr ? std::fread(buffer, elemSize, elemCount, m_pFilePtr) : 0ull;
}

int32_t CNativeInputFile::Ungetc(int32_t character)
{
	return m_pFilePtr ? std::ungetc(character, m_pFilePtr) : -1;
}

long CNativeInputFile::Tell()
{
	return m_pFilePtr ? std::ftell(m_pFilePtr) : -1;
}

int32_t CNativeInputFile::Seek(long offset, int32_t origin)
{
	return m_pFilePtr ? std::fseek(m_pFilePtr, offset, origin) : -1;
}

int32_t CNativeInputFile::Eof() const
{
	return m_pFilePtr ? std::feof(m_pFilePtr) : 1;
}