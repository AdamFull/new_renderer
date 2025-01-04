#pragma once

#include "../../gfx_enum.h"

#include <span>

namespace edge
{
	namespace gfx
	{
		namespace ext
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

EDGE_REGISTER_ENUM_CLASS(edge::gfx::ext, ShaderLang,
	EDGE_ENUM_META(edge::gfx::ext::ShaderLang::eAuto, "Auto"),
	EDGE_ENUM_META(edge::gfx::ext::ShaderLang::eGLSL, "GLSL"),
	EDGE_ENUM_META(edge::gfx::ext::ShaderLang::eHLSL, "HLSL")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx::ext, ShaderCompilerBuildType,
	EDGE_ENUM_META(edge::gfx::ext::ShaderCompilerBuildType::eDebug, "Debug"),
	EDGE_ENUM_META(edge::gfx::ext::ShaderCompilerBuildType::eExtendedDebug, "ExtDebug"),
	EDGE_ENUM_META(edge::gfx::ext::ShaderCompilerBuildType::eRelease, "Release")
)