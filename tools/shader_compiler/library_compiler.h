#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include <memory>
#include "shader_compiler.h"
#include "shader_preprocessor.h"

#include "shader_library_parse.hpp"

namespace shader_compiler
{
	class CLibraryCompiler
	{
	public:
		CLibraryCompiler() = default;
		CLibraryCompiler(const std::filesystem::path& common_path, const std::vector<std::filesystem::path>& includes_path);

		void initialize(ETargetAPIType target_api, EShaderLang shader_lang, ETargetBuildType build_type);
		void generatePermutations(const std::unordered_map<std::string, FPermutationPattern>& permutation_patterns);
		void compile(const std::filesystem::path& library_path, FShaderCacheData& shader_cache);
	private:
		std::unordered_map<std::string, std::vector<std::vector<std::string>>> m_permutations;
		std::unique_ptr<CShaderPreprocessor> m_pPreprocessor{};
		std::unique_ptr<IShaderCompiler> m_pShaderCompiler{};
	};
}