#pragma once

#include <vector>
#include <string>
#include <span>
#include <string_view>

namespace edge
{
	namespace gfx
	{
		class FilesystemWrapper
		{
		public:
			virtual ~FilesystemWrapper() = default;

			virtual bool Exists(std::string_view path) = 0;
			virtual bool Mkdir(std::string_view path) = 0;
			virtual bool ReadFile(std::string_view path, std::string& data) = 0;
			virtual bool ReadFile(std::string_view path, std::vector<uint8_t>& data) = 0;
			virtual bool WriteFile(std::string_view path, const std::vector<uint8_t>& data) = 0;
			virtual size_t FileSize(std::string_view path) = 0;
			virtual std::vector<std::string> ListDir(std::string_view path, std::span<const std::string> include_rules, std::span<const std::string> exclude_rules) = 0;
		};
	}
}