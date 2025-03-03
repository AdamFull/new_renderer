#pragma once

#include "InputFileInterface.h"
#include "VfsInputFileInterface.h"

namespace edge
{
	namespace filesystem
	{
		/**
		 * @class VFSInFile
		 * @brief A final class representing an input file in the virtual file system.
		 *
		 * VFSInFile provides file input operations, such as reading data, seeking positions,
		 * and checking the end of the file.
		 */
		class VFSInFile final : public IVFSInputFileInterface
		{
		public:
			/**
			 * @brief Default constructor.
			 */
			VFSInFile() = default;

			/**
			 * @brief Constructs a VFSInFile object with a given IInputFileInterface.
			 * @param pFile A unique_ptr to an IInputFileInterface object.
			 */
			explicit VFSInFile(std::unique_ptr<IInputFileInterface>&& pFile);

			/**
			 * @brief Destructor.
			 */
			~VFSInFile() override;

			/**
			 * @brief Checks if the input file is open.
			 * @return true if the input file is open, false otherwise.
			 */
			[[nodiscard]] auto IsOpen() -> bool override;

			/**
			 * @brief Reads a single character from the input file.
			 * @return The character read or EOF if the end of the file is reached.
			 */
			[[nodiscard]] auto Get() -> int override;

			/**
			 * @brief Reads a line of characters from the input file.
			 * @return The string containing the line read.
			 */
			[[nodiscard]] auto GetLine() -> std::string override;

			/**
			 * @brief Retrieves the number of characters read during the last unformatted input operation.
			 * @return The number of characters read.
			 */
			[[nodiscard]] auto Gcount() const ->std::size_t override;

			/**
			 * @brief Ignores a specified number of characters or until a specified delimiter is encountered.
			 * @param count The number of characters to ignore (default is 1).
			 * @param delimiter The delimiter character (default is EOF).
			 * @return A reference to the VFSInFile object.
			 */
			[[nodiscard]] auto Ignore(std::size_t count = 1, int delimiter = EOF) -> VFSInFile & override;

			/**
			 * @brief Peeks at the next character in the input file without extracting it.
			 * @return The next character in the input file or EOF if the end of the file is reached.
			 */
			[[nodiscard]] auto Peek() const -> int override;

			/**
			 * @brief Reads a specified number of characters from the input file into a buffer.
			 * @param buffer The buffer to store the characters read.
			 * @param count The number of characters to read.
			 * @return A reference to the VFSInFile object.
			 */
			[[nodiscard]] auto Read(char* buffer, std::size_t count) -> VFSInFile & override;

			/**
			 * @brief Reads characters from the input file into a buffer.
			 * @param buffer The buffer to store the characters read.
			 * @return A reference to the VFSInFile object.
			 */
			[[nodiscard]] auto Read(std::string& buffer) -> VFSInFile & override;

			/**
			 * @brief Reads characters from the input file into a buffer.
			 * @param buffer The buffer to store the characters read.
			 * @return A reference to the VFSInFile object.
			 */
			[[nodiscard]] auto Read(std::vector<uint8_t>& buffer) -> VFSInFile & override;

			/**
			 * @brief Reads characters from the input file into a buffer.
			 * @param buffer The buffer to store the characters read.
			 * @return A reference to the VFSInFile object.
			 */
			[[nodiscard]] auto Read(std::vector<uint32_t>& buffer) -> VFSInFile & override;

			/**
			 * @brief Puts the last character read back into the input file.
			 * @return A reference to the VFSInFile object.
			 */
			[[nodiscard]] auto Unget() -> VFSInFile & override;

			/**
			 * @brief Retrieves the current input position indicator.
			 * @return The current position in the input file as a std::streampos.
			 */
			[[nodiscard]] auto Tellg() -> std::streampos override;

			/**
			 * @brief Sets the input position indicator to the specified position.
			 * @param pos The new position in the input file as a std::streampos.
			 * @return A reference to the VFSInFile object.
			 */
			[[nodiscard]] auto Seekg(std::streampos pos) -> VFSInFile & override;

			/**
			 * @brief Sets the input position indicator to the specified position.
			 * @param pos The new position in the input file as a std::streampos.
			 * @param origin The origin in the input file as a C FILE seek pos.
			 * @return A reference to the VFSInFile object.
			 */
			[[nodiscard]] auto Seekg(std::streampos pos, int32_t origin) -> VFSInFile & override;

			/**
			 * @brief Checks if the end-of-file (EOF) has been reached.
			 * @return true if the end-of-file is reached, false otherwise.
			 */
			[[nodiscard]] auto Eof() const -> bool override;

		private:
			std::unique_ptr<IInputFileInterface> m_pFileInterface{ nullptr };
			std::size_t m_gcount{ 0ull };
		};
	}
}