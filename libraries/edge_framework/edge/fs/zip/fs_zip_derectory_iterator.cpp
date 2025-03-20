#include "fs_zip_derectory_iterator.h"

#include "../../util/Logger.h"

#include <zip.h>

namespace edge
{
	namespace fs
	{
		ZipDirectoryIterator::ZipDirectoryIterator(zip_t* zip, std::string_view path, bool recursive) :
			zip_handle{ zip },
			root{ path },
			recursive{ recursive }
		{
			if (!root.empty() && root.back() != '/')
				root += '/';

			entry_count = zip_get_num_entries(zip_handle, 0);

			while (current_index < entry_count) 
			{
				zip_stat_t st;
				if (zip_stat_index(zip_handle, current_index, 0, &st) == 0 && MatchesPath(st.name)) 
				{
					current_entry = st.name;
					break;
				}
				++current_index;
			}
		}

		ZipDirectoryIterator::~ZipDirectoryIterator()
		{

		}

		auto ZipDirectoryIterator::End() const noexcept -> bool
		{
			return current_index >= entry_count;
		}

		auto ZipDirectoryIterator::Next() -> void
		{
			std::lock_guard<std::mutex> lock(mutex);
			++current_index;

			while (current_index < entry_count) 
			{
				zip_stat_t st;
				if (zip_stat_index(zip_handle, current_index, 0, &st) == 0 && MatchesPath(st.name)) 
				{
					current_entry = st.name;
					break;
				}
				++current_index;
			}
		}

		auto ZipDirectoryIterator::Value() const noexcept -> std::string_view
		{
			return current_entry;
		}

		bool ZipDirectoryIterator::MatchesPath(std::string_view entry_name)
		{
			if (!entry_name.starts_with(root))
				return false;

			if (recursive)
				return true;

			// Non-recursive: exclude entries deeper than immediate children
			auto subpath = entry_name.substr(root.size());
			return subpath.find('/') == std::string_view::npos || subpath.find('/') == subpath.size() - 1;
		}
	}
}