#pragma once

#include "compiler_structures.h"
#include <hash.hpp>

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
		virtual bool preprocess(const std::string& shader_code, const FCompileInfo& info, std::string& preprocessed_code, u32& preprocessed_hash) = 0;
		virtual utl::vector<uint32_t> compile(const std::string& shader_code, const FCompileInfo& info) = 0;
	};
}