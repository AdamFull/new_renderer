#pragma once

#include "OutputFileInterface.h"
#include "VFSOutputFileInterface.h"

namespace edge
{
	namespace filesystem
	{
		/**
		 * @class VFSOutFile
		 * @brief A final class representing an output file in the virtual file system.
		 *
		 * VFSOutFile provides file output operations, such as writing data, seeking positions,
		 * and flushing the output buffer.
		 */
		class VFSOutFile final : public IVFSOutputFileInterface
		{
		public:
			/**
			 * @brief Default constructor.
			 */
			VFSOutFile() = default;

			/**
			 * @brief Constructs a VFSOutFile object with a given IOutputFileInterface.
			 * @param pFile A unique_ptr to an IOutputFileInterface object.
			 */
			explicit VFSOutFile(std::unique_ptr<IOutputFileInterface>&& pFile);

			/**
			 * @brief Destructor.
			 */
			~VFSOutFile() override;

			/**
			 * @brief Checks if the output file is open.
			 * @return true if the output file is open, false otherwise.
			 */
			[[nodiscard]] auto IsOpen() -> bool override;

			/**
			 * @brief Puts a single character into the output file.
			 * @param ch The character to be written.
			 * @return A reference to the VFSOutFile object.
			 */
			[[nodiscard]] auto Put(char ch) -> VFSOutFile & override;

			/**
			 * @brief Writes a string to the output file.
			 * @param data The string to be written.
			 * @return A reference to the VFSOutFile object.
			 */
			[[nodiscard]] auto Write(const std::string& data) -> VFSOutFile & override;

			/**
			 * @brief Writes a string to the output file.
			 * @param data The data vector to be written.
			 * @return A reference to the VFSOutFile object.
			 */
			[[nodiscard]] auto Write(const std::vector<uint8_t>& data) -> VFSOutFile & override;

			/**
			 * @brief Writes a buffer of characters to the output file.
			 * @param buffer The buffer containing the characters to be written.
			 * @param count The number of characters to write from the buffer.
			 * @return A reference to the VFSOutFile object.
			 */
			[[nodiscard]] auto Write(const char* buffer, std::size_t count) -> VFSOutFile & override;

			/**
			 * @brief Retrieves the current output position indicator.
			 * @return The current position in the output file as a std::streampos.
			 */
			[[nodiscard]] auto Tellp() -> std::streampos override;

			/**
			 * @brief Sets the output position indicator to the specified position.
			 * @param pos The new position in the output file as a std::streampos.
			 * @return A reference to the VFSOutFile object.
			 */
			[[nodiscard]] auto Seekp(std::streampos pos) -> VFSOutFile & override;

			/**
			 * @brief Flushes the output buffer.
			 * @return A reference to the VFSOutFile object.
			 */
			[[nodiscard]] auto Flush() -> VFSOutFile & override;
		private:
			std::unique_ptr<IOutputFileInterface> m_pFileInterface;
		};
	}
}