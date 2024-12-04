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

			class ShaderEffectCompiler
			{
			public:
				ShaderEffectCompiler(FilesystemWrapper* filesystem_wrapper);
				~ShaderEffectCompiler();

				void Initialize(const char* path);

			private:
				FilesystemWrapper& fs;
				ShaderEffectCompilerConfig config;
				std::unique_ptr<ShaderCompiler> shader_compiler;
			};
		}
	}
}