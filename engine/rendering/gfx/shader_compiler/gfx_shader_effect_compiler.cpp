#include "gfx_shader_effect_compiler.h"
#include <serialization.hpp>

#include "glsl_shader_compiler.h"

namespace engine
{
	namespace gfx
	{
		namespace shc
		{
			void to_json(nlohmann::json& json, const ShaderEffectCompilerPermutation& type)
			{
				utl::serialize_to("definitions", json, type.definitions);
			}

			void from_json(const nlohmann::json& json, ShaderEffectCompilerPermutation& type)
			{
				utl::parse_from("definitions", json, type.definitions);
			}


			void to_json(nlohmann::json& json, const ShaderEffectCompilerModule& type)
			{
				utl::serialize_to("name", json, type.name);
				utl::serialize_to("path", json, type.path);
				utl::serialize_to("include", json, type.include);
				utl::serialize_to("exclude", json, type.exclude);
			}

			void from_json(const nlohmann::json& json, ShaderEffectCompilerModule& type)
			{
				utl::parse_from("name", json, type.name);
				utl::parse_from("path", json, type.path);
				utl::parse_from("include", json, type.include);
				utl::parse_from("exclude", json, type.exclude);
			}


			void to_json(nlohmann::json& json, const ShaderEffectCompilerConfig& type)
			{
				utl::serialize_to("lang", json, type.lang);
				utl::serialize_to("include_directories", json, type.include_directories);
				utl::serialize_to("common_directories", json, type.common_directories);
				utl::serialize_to("permutation_patterns", json, type.permutation_patterns);
				utl::serialize_to("modules", json, type.modules);
			}

			void from_json(const nlohmann::json& json, ShaderEffectCompilerConfig& type)
			{
				utl::parse_from("lang", json, type.lang);
				utl::parse_from("include_directories", json, type.include_directories);
				utl::parse_from("common_directories", json, type.common_directories);
				utl::parse_from("permutation_patterns", json, type.permutation_patterns);
				utl::parse_from("modules", json, type.modules);
			}

			ShaderEffectCompiler::ShaderEffectCompiler(FilesystemWrapper* filesystem_wrapper) :
				fs{ *filesystem_wrapper }
			{

			}

			ShaderEffectCompiler::~ShaderEffectCompiler()
			{

			}

			void ShaderEffectCompiler::Initialize(const char* path)
			{
				std::vector<uint8_t> filedata{};
				if (fs.Exists(path) && fs.ReadFile(path, filedata))
				{
					// Load configuration
					auto json = nlohmann::json::parse(filedata);
					json.get_to(config);

					switch (config.lang)
					{
					case ShaderLang::eAuto: { assert(false && "NOT IMPLEMENTED"); } break;
					case ShaderLang::eGLSL: { shader_compiler = std::make_unique<GLSLShaderCompiler>(); } break;
					case ShaderLang::eHLSL: { assert(false && "NOT IMPLEMENTED"); } break;
					}


				}
			}
		}
	}
}