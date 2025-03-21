#pragma once

#include <string_view>
#include <vector>
#include <unordered_map>

namespace edge::gfx {
	struct ShaderCompilerCreateInfo {
		std::string_view base_path;
		std::vector<std::string_view> additional_include_dirs;
	};

	struct ShaderCompileInfo {
		std::string_view path;
		std::string_view source;
		std::vector<std::string_view> definitions;
	};

	class IShaderCompiler {
	public:
		virtual ~IShaderCompiler() = default;

		virtual auto collect_dependency_hash(const ShaderCompileInfo& compile_info) -> uint32_t = 0;
		virtual auto compile(const ShaderCompileInfo& compile_info, std::vector<uint8_t>& output) -> bool = 0;
	};
}