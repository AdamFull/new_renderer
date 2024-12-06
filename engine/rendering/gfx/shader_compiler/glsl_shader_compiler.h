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

				bool Preprocess(const CompileShaderInfo& compile_info, uint32_t& hash) override;

				bool Compile(const CompileShaderInfo& compile_info, std::vector<uint8_t>& code) override;
			private:
				FilesystemWrapper& fs;
			};
		}
	}
}