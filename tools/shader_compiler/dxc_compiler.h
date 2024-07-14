#pragma once

#include "shader_compiler.h"

#include <atlbase.h>
#include <dxcapi.h>

namespace shader_compiler
{
	class CDXCCompiler : public IShaderCompiler
	{
	public:
	public:
		CDXCCompiler(ETargetAPIType target_api, ETargetBuildType build_type);
		~CDXCCompiler();

		bool initialize() override;

		bool preprocess(std::span<uint8_t> shader_code, const FCompileInfo& info, std::string& preprocessed_code, u32& preprocessed_hash) override;
		utl::vector<uint32_t> compile(std::span<char> shader_code, const FCompileInfo& info) override;
	private:
		CComPtr<IDxcCompiler> m_pCompiler;
		CComPtr<IDxcLibrary> m_pLibrary;
	};
}