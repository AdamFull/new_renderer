#pragma once

#include <string_view>
#include <vector>
#include <unordered_map>

namespace edge
{
	namespace filesystem
	{
		class IVirtualFileSystemInterface;
	}

	namespace gfx
	{
		struct ShaderCompilerCreateInfo
		{
			filesystem::IVirtualFileSystemInterface* filesystem{ nullptr };
			std::string_view base_path;
			std::vector<std::string_view> additional_include_dirs;
		};

		struct ShaderCompileInfo
		{
			std::string_view path;
			std::string_view source;
			std::vector<std::string_view> definitions;
		};

		class IShaderCompiler
		{
		public:
			virtual ~IShaderCompiler() = default;

			virtual bool Compile(const ShaderCompileInfo& compile_info, std::vector<uint8_t>& output) = 0;
		};
	}
}