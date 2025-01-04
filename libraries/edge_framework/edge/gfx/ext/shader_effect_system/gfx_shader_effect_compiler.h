#pragma once

#include "../../gfx_common.h"
#include "gfx_shader_compiler.h"

namespace edge
{
	namespace gfx
	{
		namespace ext
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

			struct CachedShaderDesc
			{
				ShaderStageFlag stage{};
				uint32_t shader_key;
			};

			struct ShaderEffectDesc
			{
				std::vector<CachedShaderDesc> shader_descriptions;
				//std::vector<SamplerCreateInfo> sampler_create_infos;
				
			};

			struct ShaderCache
			{
				// key - shader hash, code
				std::unordered_map<uint32_t, std::string> shaders{};
				// key - shader effect unique name, (shader group, shader hash)
				std::unordered_map<uint32_t, ShaderEffectDesc> modules{};
			};

			struct ShaderEffectStage
			{
				ShaderStageFlag stage;
				std::vector<uint8_t> code;
			};

			struct ShaderEffectCreateInfo
			{
				std::vector<ShaderEffectStage> shader_stages;
				///
			};

			uint32_t GetUniqueHashName(std::string_view input);

			class ShaderEffectCompiler
			{
			public:
				ShaderEffectCompiler(const ShaderEffectCompilerCreateInfo& create_info);
				~ShaderEffectCompiler();

				void CompileModules();

				ShaderEffectCreateInfo GetEffect(std::string_view shader_name);

				void CompileOne(const char* shader_name);
			private:
				bool CompilePermutation(const CompileShaderInfo& compile_info, std::string_view shader_code);
			private:
				FilesystemWrapper& fs;
				ShaderCompilerBuildType build_type;

				ShaderEffectCompilerConfig config;
				bool is_config_loaded{ false };

				std::string base_path;
				std::string output_path;

				ShaderCache shader_cache{};

				std::unique_ptr<ShaderCompiler> shader_compiler;
			};
		}
	}
}