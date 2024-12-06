#pragma once

#include "../gfx_common.h"
#include "gfx_shader_compiler.h"

namespace engine
{
	namespace gfx
	{
		namespace shc
		{
			struct ShaderEffectCompilerPermutation
			{
				std::vector<std::string> definitions;
			};

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
				std::unordered_map<std::string, std::unordered_map<std::string, std::vector<ShaderEffectCompilerPermutation>>> permutation_patterns;
				std::vector<ShaderEffectCompilerModule> modules;
			};

			struct ShaderEffectCompilerCreateInfo
			{
				ShaderCompilerBuildType build_type{ ShaderCompilerBuildType::eDebug };
				FilesystemWrapper* filesystem_wrapper{ nullptr };
				const char* shader_compile_config_path{ nullptr };
				const char* shader_cache_patch{ nullptr };
			};

			class ShaderEffectCompiler
			{
			public:
				ShaderEffectCompiler(const ShaderEffectCompilerCreateInfo& create_info);
				~ShaderEffectCompiler();

				void CompileModules();

				void CompileOne(const char* shader_name);

			private:
				FilesystemWrapper& fs;
				ShaderCompilerBuildType build_type;

				ShaderEffectCompilerConfig config;
				bool is_config_loaded{ false };

				std::unique_ptr<ShaderCompiler> shader_compiler;
			};
		}
	}
}