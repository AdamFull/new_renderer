#include "gfx_shader_effect_compiler.h"

namespace engine
{
	namespace gfx
	{
		namespace shc
		{
			struct ShaderCompilerPermutation
			{
				std::vector<std::string> definitions;
			};

			void to_json(nlohmann::json& json, const ShaderCompilerPermutation& type)
			{

			}

			void from_json(const nlohmann::json& json, ShaderCompilerPermutation& type)
			{

			}

			struct ShaderCompilerModule
			{
				std::string name;
				std::string path;
				std::vector<std::string> include;
				std::vector<std::string> exclude;
			};

			void to_json(nlohmann::json& json, const ShaderCompilerModule& type)
			{

			}

			void from_json(const nlohmann::json& json, ShaderCompilerModule& type)
			{

			}

			struct ShaderEffectCompilerConfig
			{
				ShaderLang lang;
				std::vector<std::string> include_directories;
				std::vector<std::string> common_directories;
				std::unordered_map<std::string, std::unordered_map<std::string, std::vector<ShaderCompilerPermutation>>> permutation_patterns;
				std::vector<ShaderCompilerModule> modules;
			};

			void to_json(nlohmann::json& json, const ShaderEffectCompilerConfig& type)
			{

			}

			void from_json(const nlohmann::json& json, ShaderEffectCompilerConfig& type)
			{

			}

			ShaderEffectCompiler::ShaderEffectCompiler(FilesystemWrapper* filesystem_wrapper) :
				fs{ *filesystem_wrapper }
			{

			}

			ShaderEffectCompiler::~ShaderEffectCompiler()
			{

			}
		}
	}
}