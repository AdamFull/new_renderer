#pragma once

#include <cstdint>
#include <span>

#include "../util/Enum.hpp"

namespace edge::fs {
	enum class FileModeFlag {
		eRead = 1 << 0,
		eWrite = 1 << 1,
		eBin = 1 << 2,
		eAppend = 1 << 3
	};
}

EDGE_REGISTER_ENUM_FLAGS(edge::fs, FileModeFlags, FileModeFlag,
	EDGE_ENUM_META(edge::fs::FileModeFlag::eRead, "r"),
	EDGE_ENUM_META(edge::fs::FileModeFlag::eWrite, "w"),
	EDGE_ENUM_META(edge::fs::FileModeFlag::eBin, "b"),
	EDGE_ENUM_META(edge::fs::FileModeFlag::eAppend, "a")
)

namespace edge::fs {
	enum class SeekOrigin {
		eSet,
		eCur,
		eEnd
	};

	class IFile {
	public:
		virtual ~IFile() = default;

		[[nodiscard]] virtual auto is_open() -> bool = 0;
		virtual auto close() -> void = 0;

		[[nodiscard]] virtual auto size() -> uint64_t = 0;
		virtual auto seek(uint64_t offset, SeekOrigin origin) -> int64_t = 0;
		[[nodiscard]] virtual auto tell() -> int64_t = 0;
		virtual auto read(void* buffer, uint64_t size) -> int64_t = 0;
		virtual auto write(const void* buffer, uint64_t size) -> int64_t = 0;

		template<typename _Ty>
		auto read(_Ty& value) -> uint64_t {
			return read(&value, sizeof(_Ty));
		}

		template<>
		auto read(std::string& buffer) -> uint64_t {
			return read(buffer.data(), buffer.size());
		}

		template<>
		auto read(std::vector<uint8_t>& buffer) -> uint64_t {
			return read(buffer.data(), buffer.size());
		}

		auto read(std::span<uint8_t> buffer) -> uint64_t {
			return read(buffer.data(), buffer.size());
		}

		template<typename _Ty>
		auto write(const _Ty& value) -> uint64_t {
			return write(&value, sizeof(_Ty));
		}

		template<>
		auto write(const std::string& buffer) -> uint64_t {
			return write(buffer.data(), buffer.size());
		}

		template<>
		auto write(const std::vector<uint8_t>& buffer) -> uint64_t {
			return write(buffer.data(), buffer.size());
		}
	};
}