#pragma once

#include "../../util/fs_helper.h"
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
				this->path = fsh::to_posix_path(std::filesystem::relative(iterator->path(), root).string(), buffer);
			}

			auto End() -> bool override
			{
				return iterator == _Ty();
			}

			auto Next() -> void override
			{
				++iterator;
				if (iterator != _Ty())
				{
					std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
					auto decoded = convert.to_bytes(std::filesystem::relative(iterator->path(), root).u16string());

					std::array<char, 1024ull> buffer{};
					path = fsh::to_posix_path(decoded, buffer);
				}
			}

			auto Value() -> const std::string& override
			{
				return path;
			}

			_Ty iterator;
			std::string path;
			std::string root;
		};
	}
}