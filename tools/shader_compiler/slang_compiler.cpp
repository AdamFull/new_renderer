#include "slang_compiler.h"

namespace shader_compiler
{
	SLangGCompiler::SLangGCompiler(ETargetAPIType target_api, ETargetBuildType build_type)
	{

	}

	SLangGCompiler::~SLangGCompiler()
	{

	}

	bool SLangGCompiler::initialize()
	{
		return false;
	}

	bool SLangGCompiler::preprocess(const std::string& shader_code, const FCompileInfo& info, std::string& preprocessed_code, u32& preprocessed_hash)
	{
		return false;
	}

	utl::vector<uint32_t> SLangGCompiler::compile(const std::string& shader_code, const FCompileInfo& info)
	{
		return {};
	}
}