#pragma once

#include "compiler_structures.h"
#include <utils/hash.h>

namespace shader_compiler
{
	struct FCompileInfo
	{
		EShaderType type;
		std::string name;
	};

	class IShaderCompiler
	{
	public:
		virtual ~IShaderCompiler() = default;

		virtual bool initialize() = 0;
		virtual bool preprocess(const std::string& shader_code, const FCompileInfo& info, std::string& preprocessed_code, size_t& preprocessed_hash) = 0;
		virtual utl::vector<uint32_t> compile(const std::string& shader_code, const FCompileInfo& info) = 0;
	};
}