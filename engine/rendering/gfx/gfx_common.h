#pragma once

#include <vector>

namespace engine
{
	namespace gfx
	{
		class FilesystemWrapper
		{
		public:
			virtual ~FilesystemWrapper() = default;

			virtual bool Exists(const char* path) = 0;
			virtual bool ReadFile(const char* path, std::vector<uint8_t>& data) = 0;
			virtual bool WriteFile(const char* path, const std::vector<uint8_t>& data) = 0;
			virtual size_t FileSize(const char* path) = 0;
		};
	}
}