#pragma once

#include "../gfx_common.h"
#include "gfx_shader_compiler.h"

namespace engine
{
	namespace gfx
	{
		namespace shc
		{
			class ShaderEffectCompiler
			{
			public:
				ShaderEffectCompiler(FilesystemWrapper* filesystem_wrapper);
				~ShaderEffectCompiler();

			private:
				FilesystemWrapper& fs;
				std::unique_ptr<ShaderCompiler> shader_compiler;
			};
		}
	}
}