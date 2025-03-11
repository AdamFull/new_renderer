#include "fs_zip_derectory_iterator.h"

#include "../../util/Logger.h"

#include <zip.h>

namespace edge
{
	namespace fs
	{
		ZipDirectoryIterator::ZipDirectoryIterator(zip_t* zip, std::string_view path, bool recursive) :
			zip_handle{ zip }
		{
			for (zip_int64_t i = 0; i < zip_get_num_entries(zip_handle, 0); ++i)
			{
				zip_stat_t stat;
				if (zip_stat_index(zip_handle, i, 0, &stat) == 0)
				{
					std::string entry_path(stat.name);

					if (entry_path == path)
						continue;

					if (recursive)
					{
						if (entry_path.find(path) == 0)
							entries.emplace_back(entry_path);
					}
					else
					{
						if (entry_path.find(path) == 0 && entry_path.find('/', path.size()) == std::string::npos)
							entries.emplace_back(entry_path);
					}
				}
				else
				{
					EDGE_LOGE("[ZipDirectoryIterator] Failed to read entry information in: {}, entry: {}", path, i);
				}
			}
		}

		ZipDirectoryIterator::~ZipDirectoryIterator()
		{

		}

		auto ZipDirectoryIterator::End() -> bool
		{
			return current_entry == entries.size();
		}

		auto ZipDirectoryIterator::Next() -> void
		{
			++current_entry;
		}

		auto ZipDirectoryIterator::Value() -> const std::string&
		{
			return entries[current_entry];
		}
	}
}