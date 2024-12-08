#pragma once

#include "gfx_shader_compiler.h"
#include "../gfx_common.h"

namespace engine
{
	namespace gfx
	{
		namespace shc
		{
			class GLSLShaderCompiler : public ShaderCompiler
			{
			public:
				GLSLShaderCompiler(FilesystemWrapper& fs);
				~GLSLShaderCompiler();

				bool Preprocess(const CompileShaderInfo& compile_info, std::string_view source_code, std::string& preprocessed_code, uint32_t& hash) override;

				bool Compile(const CompileShaderInfo& compile_info, std::string_view source_code, std::vector<uint8_t>& code) override;
			private:
				FilesystemWrapper& fs;
			};
		}
	}
}