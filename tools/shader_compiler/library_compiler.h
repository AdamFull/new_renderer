#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <memory>
#include "shader_compiler.h"

#include "shader_library_parse.hpp"

namespace shader_compiler
{
	class CLibraryCompiler
	{
	public:
		CLibraryCompiler() = default;
		CLibraryCompiler(const std::filesystem::path& common_path, const utl::vector<std::filesystem::path>& includes_path);

		void Initialize(ETargetAPIType target_api, EShaderLang shader_lang, ETargetBuildType build_type);
		void GeneratePermutations(const utl::hash_map<std::string, FPermutationPattern>& permutation_patterns);
		void Compile(const std::filesystem::path& library_path, FShaderCacheData& shader_cache);
	protected:
		bool LoadShader(const std::filesystem::path& library_path, const std::string& meta_name, FShaderMeta& meta, std::vector<uint8_t>& code);
		std::string GeneratePreamble(const FShaderMeta& meta);
		void CompilePermutations(const std::filesystem::path& parent_path, const std::string& name, const utl::vector<std::string>& sources, const utl::vector<utl::vector<std::string>>& cached_permutations, FCompiledLibrary& compiled_library);
	private:
		utl::hash_map<std::string, utl::vector<utl::vector<std::string>>> m_permutations;
		std::unique_ptr<IShaderCompiler> m_pShaderCompiler{};
		std::array<std::mutex, 4> m_locks{};

		std::filesystem::path m_commonPath;
		utl::vector<std::filesystem::path> m_vIncludeDirs;

		EShaderLang m_lang;
	};
}