#pragma once

#include "../fs_helper.h"
#include "../fs_directory_iterator.h"

#include <array>
#include <string>
#include <codecvt>

namespace edge
{
	namespace fs
	{
		template<class _Ty>
		class NativeDirectoryIterator final : public IDirectoryIterator
		{
		public:
			NativeDirectoryIterator() = default;

			NativeDirectoryIterator(std::string_view path, std::string_view root) :
				iterator(_Ty(path))
			{
				this->root = root;

				std::array<char, 1024ull> buffer{};
				this->path = fs::to_posix_path(std::filesystem::relative(iterator->path(), root).string(), buffer);
			}

			auto end() const noexcept -> bool override
			{
				return iterator == _Ty();
			}

			auto next() -> void override
			{
				++iterator;
				if (iterator != _Ty())
				{
					std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
					auto decoded = convert.to_bytes(std::filesystem::relative(iterator->path(), root).u16string());

					std::array<char, 1024ull> buffer{};
					path = fs::to_posix_path(decoded, buffer);
				}
			}

			auto value() const noexcept -> std::string_view override
			{
				return path;
			}

			_Ty iterator;
			std::string path;
			std::string root;
		};
	}
}