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

				ShaderStageFlag stage;

				std::string_view base_path;

				std::string_view module_name;
				std::string_view name;

				std::span<const std::string> base_definitions;
				std::span<const std::string> definitions;

				std::span<const std::string> include_directories;

				uint32_t shader_index{ 0u };
			};

			class ShaderCompiler
			{
			public:
				virtual ~ShaderCompiler() = default;

				virtual bool Preprocess(const CompileShaderInfo& compile_info, std::string_view source_code, std::string& preprocessed_code, uint32_t& hash) = 0;

				virtual bool Compile(const CompileShaderInfo& compile_info, std::string_view source_code, std::vector<uint8_t>& code) = 0;
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