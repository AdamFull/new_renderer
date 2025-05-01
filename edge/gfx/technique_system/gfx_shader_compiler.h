#pragma once

#include <string_view>
#include <vector>
#include <unordered_map>

#include "../gfx_shader_reflection.h"

namespace edge::gfx {
	struct ShaderCompilerCreateInfo {
		std::string_view base_path;
		std::vector<std::string_view> additional_include_dirs;
	};

	struct ShaderCompileInfo {
		std::string_view path;
		std::string_view source;
		std::vector<std::string> definitions;
	};

	class ICompilationResult {
	public:
		virtual ~ICompilationResult() = default;

		virtual auto collect_result(std::vector<uint8_t>& output) -> bool = 0;
		virtual auto collect_reflection(ShaderReflection& refl) -> bool = 0;
	};

	class IShaderCompiler {
	public:
		virtual ~IShaderCompiler() = default;

		virtual auto collect_dependency_hash(const ShaderCompileInfo& compile_info) -> uint32_t = 0;
		virtual auto compile(const ShaderCompileInfo& compile_info) -> ICompilationResult* = 0;
	};
}