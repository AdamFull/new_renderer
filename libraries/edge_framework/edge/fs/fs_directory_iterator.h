#pragma once

#include <string>

namespace edge
{
	namespace fs
	{
		class IDirectoryIterator
		{
		public:
			virtual ~IDirectoryIterator() = default;

			virtual auto End() -> bool = 0;
			virtual auto Next() -> void = 0;
			virtual auto Value() -> const std::string & = 0;
		};
	}
}