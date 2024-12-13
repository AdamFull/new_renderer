#include "NativeOutputFile.h"

#include <stdexcept>

namespace edge
{
	namespace filesystem
	{
		NativeOutputFile::NativeOutputFile(std::string_view filename)
		{
			m_pFilePtr = std::fopen(filename.data(), "wb");

			if (!m_pFilePtr)
				throw std::runtime_error("Failed to open native file.");
		}

		NativeOutputFile::~NativeOutputFile()
		{
			Close();
		}

		void NativeOutputFile::Close()
		{
			if (m_pFilePtr)
			{
				fclose(m_pFilePtr);
				m_pFilePtr = nullptr;
			}
		}

		bool NativeOutputFile::IsOpen() const
		{
			return m_pFilePtr;
		}

		int32_t NativeOutputFile::Putc(int32_t character)
		{
			return m_pFilePtr ? std::fputc(character, m_pFilePtr) : -1;
		}

		std::size_t NativeOutputFile::Write(const void* buffer, std::size_t elemSize, std::size_t elemCount)
		{
			return m_pFilePtr ? std::fwrite(buffer, elemSize, elemCount, m_pFilePtr) : 0ull;
		}

		long NativeOutputFile::Tell()
		{
			return m_pFilePtr ? std::ftell(m_pFilePtr) : -1;
		}

		int32_t NativeOutputFile::Seek(long offset, int32_t origin)
		{
			return m_pFilePtr ? std::fseek(m_pFilePtr, offset, origin) : -1;
		}

		int32_t NativeOutputFile::Flush()
		{
			return m_pFilePtr ? std::fflush(m_pFilePtr) : -1;
		}
	}
}