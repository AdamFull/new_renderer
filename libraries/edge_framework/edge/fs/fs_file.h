#pragma once

#include <cstdint>
#include <span>

#include "../util/Enum.hpp"

namespace edge
{
	namespace fs
	{
		enum class FileModeFlag
		{
			eRead = 1 << 0,
			eWrite = 1 << 1,
			eBin = 1 << 2, 
			eAppend = 1 << 3
		};
	}
}

EDGE_REGISTER_ENUM_FLAGS(edge::fs, FileModeFlags, FileModeFlag,
	EDGE_ENUM_META(edge::fs::FileModeFlag::eRead, "r"),
	EDGE_ENUM_META(edge::fs::FileModeFlag::eWrite, "w"),
	EDGE_ENUM_META(edge::fs::FileModeFlag::eBin, "b"),
	EDGE_ENUM_META(edge::fs::FileModeFlag::eAppend, "a")
)

namespace edge
{
	namespace fs
	{
		enum class SeekOrigin
		{
			eSet,
			eCur,
			eEnd
		};

		class IFile
		{
		public:
			virtual ~IFile() = default;

			[[nodiscard]] virtual auto IsOpen() -> bool = 0;
			virtual auto Close() -> void = 0;

			[[nodiscard]] virtual auto Seek(uint64_t offset, SeekOrigin origin) -> uint64_t = 0;
			[[nodiscard]] virtual uint64_t Tell() = 0;
			[[nodiscard]] virtual uint64_t Read(void* buffer, uint64_t size) = 0;
			[[nodiscard]] virtual uint64_t Write(const void* buffer, uint64_t size) = 0;

            template<typename _Ty>
            auto Read(_Ty& value) -> uint64_t
            {
                return Read(&value, sizeof(_Ty));
            }

			template<>
			auto Read(std::string& buffer) -> uint64_t
			{
				return Read(buffer.data(), buffer.size());
			}

			template<>
			auto Read(std::vector<uint8_t>& buffer) -> uint64_t
			{
				return Read(buffer.data(), buffer.size());
			}

			auto Read(std::span<uint8_t> buffer) -> uint64_t
			{
				return Read(buffer.data(), buffer.size());
			}

            template<typename _Ty>
            auto Write(const _Ty& value) -> uint64_t
            {
                return Write(&value, sizeof(_Ty));
            }

			template<>
			auto Write(const std::string& buffer) -> uint64_t
			{
				return Write(buffer.data(), buffer.size());
			}

			template<>
			auto Write(const std::vector<uint8_t>& buffer) -> uint64_t
            {
                return Write(buffer.data(), buffer.size());
            }
		};
	}
}