#include "gfx_shader_effect_compiler.h"
#include <serialization.hpp>

#include "glsl_shader_compiler.h"

#include <util/FilesystemHelper.h>
#include <util/Logger.h>

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

			ShaderEffectCompiler::ShaderEffectCompiler(const ShaderEffectCompilerCreateInfo& create_info) :
				fs{ *create_info.filesystem_wrapper },
				build_type{ create_info.build_type }
			{
				// Load compiler config
				std::vector<uint8_t> filedata{};
				if (!create_info.shader_compile_config_path.empty() &&
					fs.Exists(create_info.shader_compile_config_path) && 
					fs.ReadFile(create_info.shader_compile_config_path, filedata))
				{
					// Load configuration
					auto json = nlohmann::json::parse(filedata);
					json.get_to(config);

					is_config_loaded = true;
					base_path = fsh::parent_path(create_info.shader_compile_config_path);

					switch (config.lang)
					{
					case ShaderLang::eAuto: { assert(false && "NOT IMPLEMENTED"); } break;
					case ShaderLang::eGLSL: { shader_compiler = std::make_unique<GLSLShaderCompiler>(); } break;
					case ShaderLang::eHLSL: { assert(false && "NOT IMPLEMENTED"); } break;
					}

					filedata.clear();
				}

				// Load compiler cache
				if (!create_info.shader_cache_patch.empty() &&
					fs.Exists(create_info.shader_cache_patch) && 
					fs.ReadFile(create_info.shader_cache_patch, filedata))
				{

				}
			}

			ShaderEffectCompiler::~ShaderEffectCompiler()
			{

			}

			void ShaderEffectCompiler::CompileModules()
			{
				if (!is_config_loaded)
				{
					log_error("Compiler config was not loaded. Unable to compile modules.");
					return;
				}

				std::array<char, 1024ull> str_buf{};

				for (const auto& compile_module : config.modules)
				{
					log_info("Building module: {}", compile_module.name);

					auto module_dir = fsh::path_append(base_path, compile_module.path, str_buf);
				}
			}

			void ShaderEffectCompiler::CompileOne(const char* shader_name)
			{
				if (!is_config_loaded)
				{
					log_error("Compiler config was not loaded. Unable to compile shaders.");
					return;
				}
			}
		}
	}
}