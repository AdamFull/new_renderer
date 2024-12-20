#include "VFSOutputFile.h"

#include <stdexcept>

namespace edge
{
	namespace filesystem
	{
		VFSOutFile::VFSOutFile(std::unique_ptr<IOutputFileInterface>&& pFile) :
			m_pFileInterface(std::move(pFile))
		{
			if (!m_pFileInterface)
				throw std::runtime_error("Failed to create file");
		}

		VFSOutFile::~VFSOutFile()
		{
			if (m_pFileInterface)
				m_pFileInterface->Close();
		}

		bool VFSOutFile::IsOpen()
		{
			if (m_pFileInterface)
				return m_pFileInterface->IsOpen();

			return false;
		}

		VFSOutFile& VFSOutFile::Put(char ch)
		{
			if (m_pFileInterface)
				m_pFileInterface->Putc(ch);

			return *this;
		}

		VFSOutFile& VFSOutFile::Write(const std::string& str)
		{
			if (m_pFileInterface)
				m_pFileInterface->Write(str.c_str(), sizeof(char), str.size());

			return *this;
		}

		VFSOutFile& VFSOutFile::Write(const std::vector<uint8_t>& data)
		{
			if (m_pFileInterface)
				m_pFileInterface->Write(data.data(), sizeof(uint8_t), data.size());

			return *this;
		}

		VFSOutFile& VFSOutFile::Write(const char* buffer, std::size_t count)
		{
			if (m_pFileInterface)
				m_pFileInterface->Write(buffer, sizeof(char), count);

			return *this;
		}

		std::streampos VFSOutFile::Tellp()
		{
			return m_pFileInterface ? m_pFileInterface->Tell() : -1;
		}

		VFSOutFile& VFSOutFile::Seekp(std::streampos pos)
		{
			if (m_pFileInterface)
				m_pFileInterface->Seek(pos, SEEK_SET);

			return *this;
		}

		VFSOutFile& VFSOutFile::Flush()
		{
			if (m_pFileInterface)
				m_pFileInterface->Flush();

			return *this;
		}
	}
}