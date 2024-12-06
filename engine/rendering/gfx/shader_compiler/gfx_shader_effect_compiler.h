#pragma once

#include "../gfx_common.h"
#include "gfx_shader_compiler.h"

namespace engine
{
	namespace gfx
	{
		namespace shc
		{
			struct ShaderEffectCompilerModule
			{
				std::string name;
				std::string path;
				std::vector<std::string> include;
				std::vector<std::string> exclude;
			};

			struct ShaderEffectCompilerConfig
			{
				ShaderLang lang;
				std::vector<std::string> include_directories;
				std::vector<std::string> common_directories;
				std::unordered_map<std::string, std::vector<std::vector<std::string>>> permutation_patterns;
				std::vector<ShaderEffectCompilerModule> modules;
			};

			struct ShaderEffectCompilerCreateInfo
			{
				ShaderCompilerBuildType build_type{ ShaderCompilerBuildType::eDebug };
				FilesystemWrapper* filesystem_wrapper{ nullptr };
				std::string_view shader_compile_config_path{ "" };
				std::string_view shader_cache_patch{ "" };
			};

			class ShaderEffectCompiler
			{
			public:
				ShaderEffectCompiler(const ShaderEffectCompilerCreateInfo& create_info);
				~ShaderEffectCompiler();

				void CompileModules();

				void CompileOne(const char* shader_name);
			private:
				void CompilePermutation(const CompileShaderInfo& compile_info);
			private:
				FilesystemWrapper& fs;
				ShaderCompilerBuildType build_type;

				ShaderEffectCompilerConfig config;
				bool is_config_loaded{ false };
				std::string base_path;

				std::unique_ptr<ShaderCompiler> shader_compiler;
			};
		}
	}
}