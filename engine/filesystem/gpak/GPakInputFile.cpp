#include "GPakInputFile.h"

#include <stdexcept>

#include <gpak.h>

using namespace engine::filesystem;

CGPakInputFile::CGPakInputFile(gpak* handle, const std::string& filename)
{
	if (!handle)
		return;

	m_pFilePtr = gpak_fopen(handle, filename.c_str());

	if (!m_pFilePtr)
		throw std::runtime_error("Failed to open gpak file.");
}

CGPakInputFile::~CGPakInputFile()
{
	Close();
}

void CGPakInputFile::Close()
{
	if (m_pFilePtr)
	{
		gpak_fclose(m_pFilePtr);
		m_pFilePtr = nullptr;
	}
}

bool CGPakInputFile::IsOpen() const
{
	return m_pFilePtr;
}

int32_t CGPakInputFile::Getc()
{
	return m_pFilePtr ? gpak_fgetc(m_pFilePtr) : -1;
}

std::size_t CGPakInputFile::Read(void* buffer, std::size_t elemSize, std::size_t elemCount)
{
	return m_pFilePtr ? gpak_fread(buffer, elemSize, elemCount, m_pFilePtr) : 0ull;
}

int32_t CGPakInputFile::Ungetc(int32_t character)
{
	return m_pFilePtr ? gpak_ungetc(m_pFilePtr, character) : -1;
}

long CGPakInputFile::Tell()
{
	return m_pFilePtr ? gpak_ftell(m_pFilePtr) : -1;
}

int32_t CGPakInputFile::Seek(long offset, int32_t origin)
{
	return m_pFilePtr ? gpak_fseek(m_pFilePtr, offset, origin) : -1;
}

int32_t CGPakInputFile::Eof() const
{
	return m_pFilePtr ? std::feof(m_pFilePtr->stream_) : 1;
}