#pragma once

#include "../fs_directory_iterator.h"

#include <vector>
#include <string>
#include <string_view>

typedef struct zip zip_t;

namespace edge
{
	namespace fs
	{
		class ZipDirectoryIterator final : public IDirectoryIterator
		{
		public:
			ZipDirectoryIterator(zip_t* zip, std::string_view path, bool recursive);
			~ZipDirectoryIterator() override;

			auto End() -> bool override;
			auto Next() -> void override;
			auto Value() -> const std::string & override;
		private:
			zip_t* zip_handle{ nullptr };

			int32_t current_entry{ 0 };
			std::vector<std::string> entries{};
		};
	}
}