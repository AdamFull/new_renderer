#pragma once

#include "../gfx_enum.h"

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

			enum class ShaderCompilerBuildType
			{
				eDebug,
				eExtendedDebug,
				eRelease
			};

			struct CompileShaderInfo
			{
				ShaderCompilerBuildType build_type;
				ShaderStageFlag type;
				std::string name;
				std::string code;
				std::vector<std::string> definitions;
				std::vector<std::string> include_directories;
			};

			class ShaderCompiler
			{
			public:
				virtual ~ShaderCompiler() = default;

				virtual bool Preprocess(const CompileShaderInfo& compile_info) = 0;

				virtual bool Compile(const CompileShaderInfo& compile_info) = 0;
			};
		}
	}
}

UTL_REGISTER_ENUM_CLASS(engine::gfx::shc, ShaderLang,
	UTL_ENUM_META(engine::gfx::shc::ShaderLang::eAuto, "Auto"),
	UTL_ENUM_META(engine::gfx::shc::ShaderLang::eGLSL, "GLSL"),
	UTL_ENUM_META(engine::gfx::shc::ShaderLang::eHLSL, "HLSL")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx::shc, ShaderCompilerBuildType,
	UTL_ENUM_META(engine::gfx::shc::ShaderCompilerBuildType::eDebug, "Debug"),
	UTL_ENUM_META(engine::gfx::shc::ShaderCompilerBuildType::eExtendedDebug, "ExtDebug"),
	UTL_ENUM_META(engine::gfx::shc::ShaderCompilerBuildType::eRelease, "Release")
)