#pragma once

#include <enum.hpp>

namespace engine
{
	namespace gfx
	{
		namespace shc
		{
			enum class ShaderLang
			{
				eAuto,
				eGLSL,
				eHLSL
			};

			class ShaderCompiler
			{
			public:
				virtual ~ShaderCompiler() = default;

				virtual bool preprocess() = 0;

				virtual bool compile() = 0;
			};
		}
	}
}

UTL_REGISTER_ENUM_CLASS(engine::gfx::shc, ShaderLang,
	UTL_ENUM_META(engine::gfx::shc::ShaderLang::eAuto, "Auto"),
	UTL_ENUM_META(engine::gfx::shc::ShaderLang::eGLSL, "GLSL"),
	UTL_ENUM_META(engine::gfx::shc::ShaderLang::eHLSL, "HLSL")
)