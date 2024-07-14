#pragma once

#include "compiler_structures.h"
#include <utils/hash.h>

namespace shader_compiler
{
	struct FCompileInfo
	{
		i32 version;
		EShaderType type;
		std::string name;
		std::string preamble;
		std::span<std::filesystem::path> include_directories;
	};

	class IShaderCompiler
	{
	public:
		virtual ~IShaderCompiler() = default;

		virtual bool initialize() = 0;
		virtual bool preprocess(std::span<uint8_t> shader_code, const FCompileInfo& info, std::string& preprocessed_code, u32& preprocessed_hash) = 0;
		virtual utl::vector<uint32_t> compile(std::span<char> shader_code, const FCompileInfo& info) = 0;
	};
}