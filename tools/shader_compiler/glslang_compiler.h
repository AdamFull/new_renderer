#pragma once

#include "shader_compiler.h"
#include <mutex>

namespace shader_compiler
{
	class CGLSLANGCompiler : public IShaderCompiler
	{
	public:
		CGLSLANGCompiler(ETargetAPIType target_api, ETargetBuildType build_type);
		~CGLSLANGCompiler();

		bool initialize() override;

		bool preprocess(const std::string& shader_code, const FCompileInfo& info, std::string& preprocessed_code, size_t& preprocessed_hash) override;
		utl::vector<uint32_t> compile(const std::string& shader_code, const FCompileInfo& info) override;
	private:
		ETargetBuildType build_type{ ETargetBuildType::eRelease };
		ETargetAPIType target_api{ ETargetAPIType::eVK13 };
		std::mutex m_terminationLock{};
	};
}