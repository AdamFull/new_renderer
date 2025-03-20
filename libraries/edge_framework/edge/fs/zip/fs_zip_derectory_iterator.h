#pragma once

#include "../fs_directory_iterator.h"

#include <vector>
#include <string>
#include <string_view>
#include <mutex>

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

			auto end() const noexcept -> bool override;
			auto next() -> void override;
			auto value() const noexcept -> std::string_view override;
		private:
			bool matches_path(std::string_view entry_name);

			zip_t* zip_handle{ nullptr };

			std::string root{};
			bool recursive{ false };

			int32_t current_index{ 0 };
			int32_t entry_count{ 0 };

			std::string_view current_entry;

			std::mutex mutex;
		};
	}
}