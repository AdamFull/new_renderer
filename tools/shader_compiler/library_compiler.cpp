#include "library_compiler.h"

#include "glslang_compiler.h"
#include "dxc_compiler.h"

#include "compiler_util.h"
#include "compiler_parse.h"
#include "shader_library_parse.hpp"

#include <fstream>
#include <regex>
#include <execution>

using namespace shader_compiler;

std::string get_definition_by_shader_type(EShaderType type)
{
	std::string res{};

	switch (type)
	{
	case shader_compiler::EShaderType::eUnknown: res = "INVALID_SHADER"; break;
	case shader_compiler::EShaderType::eVertex: res = "VERTEX_SHADER"; break;
	case shader_compiler::EShaderType::eControl: res = "TESS_CONTROL_SHADER"; break;
	case shader_compiler::EShaderType::eEvaluation: res = "TESS_EVAL_SHADER"; break;
	case shader_compiler::EShaderType::eGeometry: res = "GEOMETRY_SHADER"; break;
	case shader_compiler::EShaderType::eFragment: res = "FRAGMENT_SHADER"; break;
	case shader_compiler::EShaderType::eCompute: res = "COMPUTE_SHADER"; break;
	case shader_compiler::EShaderType::eRayGen: res = "RAY_GEN_SHADER"; break;
	case shader_compiler::EShaderType::eIntersection: res = "INTERSECTION_SHADER"; break;
	case shader_compiler::EShaderType::eAnyHit: res = "ANY_HIT_SHADER"; break;
	case shader_compiler::EShaderType::eClosestHit: res = "CLOSEST_HIT_SHADER"; break;
	case shader_compiler::EShaderType::eMiss: res = "MISS_SHADER"; break;
	case shader_compiler::EShaderType::eCallable: res = "CALLABLE_SHADER"; break;
	case shader_compiler::EShaderType::eTask: res = "TASK_SHADER"; break;
	case shader_compiler::EShaderType::eMesh: res = "MESH_SHADER"; break;
	}

	return res;
}

std::string emplace_data(std::string base_string, const std::string& target, const std::string& replacement)
{
	u64 start_pos = base_string.find(target);
	if (start_pos == std::string::npos)
		return base_string;
	base_string.replace(start_pos, target.length(), replacement);
	return base_string;
}

//utl::vector<std::string> make_permutation(u64 mask, const utl::vector<FDefinitionChain>& chains) 
//{
//	utl::vector<std::string> permutation;
//
//	for (u64 j = 0; j < chains.size(); ++j) 
//	{
//		if (mask & (1 << j)) 
//		{
//			permutation.emplace_back(chains[j].base);
//
//			if (!chains[j].var.empty())
//				permutation.emplace_back(chains[j].var[0].base);
//		}
//	}
//
//	return permutation;
//}

void generate_permutations(const utl::vector<std::string>& base, const utl::vector<FDefinitionVariant>& variants, utl::vector<utl::vector<std::string>>& result, utl::vector<std::string> current, size_t variant_index)
{
	if (variant_index == variants.size()) 
	{
		result.emplace_back(current);
		return;
	}

	const auto& variant = variants[variant_index];
	if (variant.type == EPermutationVariantCombineType::eAND)
	{
		current.insert(current.end(), variant.definitions.begin(), variant.definitions.end());
		generate_permutations(base, variants, result, current, variant_index + 1);
	}
	else if (variant.type == EPermutationVariantCombineType::eOR) 
	{
		for (const auto& definition : variant.definitions) 
		{
			utl::vector<std::string> new_current = current;

			if(!definition.empty())
				new_current.emplace_back(definition);

			generate_permutations(base, variants, result, new_current, variant_index + 1);
		}
	}
}

CLibraryCompiler::CLibraryCompiler(const std::filesystem::path& common_path, const utl::vector<std::filesystem::path>& includes_path)
{
	m_pPreprocessor = std::make_unique<CShaderPreprocessor>(common_path, includes_path);
}


void CLibraryCompiler::initialize(ETargetAPIType target_api, EShaderLang shader_lang, ETargetBuildType build_type)
{
	switch (shader_lang)
	{
	case shader_compiler::EShaderLang::eUnknown: break;
	case shader_compiler::EShaderLang::eGLSL: { m_pShaderCompiler = std::make_unique<CGLSLANGCompiler>(target_api, build_type); } break;
	case shader_compiler::EShaderLang::eHLSL: { m_pShaderCompiler = std::make_unique<CDXCCompiler>(target_api, build_type); } break;
	}

	m_pPreprocessor->initialize(shader_lang);
	m_pShaderCompiler->initialize();
}

void CLibraryCompiler::generatePermutations(const utl::hash_map<std::string, FPermutationPattern>& permutation_patterns)
{
	for (const auto& [name, pattern] : permutation_patterns)
	{
		auto& new_permutations = m_permutations[name];

		// At first copy default permutations
		for (const auto& def_list : pattern.permutations)
			new_permutations.emplace_back(def_list.definitions);

		// Generate another permutations
		for (const auto& rule : pattern.generator.rules)
		{
			// If we don't have any variants, just create with base
			if (rule.variants.empty())
			{
				auto& new_permutation = new_permutations.emplace_back();
				new_permutation.insert(new_permutation.end(), rule.base.begin(), rule.base.end());
				continue;
			}

			generate_permutations(rule.base, rule.variants, new_permutations, rule.base, 0);
		}
	}
}

void CLibraryCompiler::compile(const std::filesystem::path& library_path, FShaderCacheData& shader_cache)
{
	FShaderLibrary shader_library_ci{};
	if (!parse_file_to(library_path, shader_library_ci))
		return;

	FCompiledLibrary& compiled_library = shader_cache.libraries[utl::fnv1a_32_hash_cstr(shader_library_ci.name.c_str())];

	auto parent_path = library_path.parent_path();

	// Process compile all instructions from library file
	for (auto& compile : shader_library_ci.compiles)
	{
		auto found_shader_desc = shader_library_ci.shaders.find(compile.name);
		if (found_shader_desc == shader_library_ci.shaders.end())
		{
			log_error("Trying to compile shader {}, but shader description for this shader was not declared in \"shaders\" scope.", compile.name);
			continue;
		}

		FShaderDescription& shader_desc = found_shader_desc->second;

		// Get cached permutations
		if (!compile.pattern.empty())
		{
			const auto& cached_permutations = m_permutations[compile.pattern];
			compilePermutations(parent_path, compile.name, shader_desc.sources, cached_permutations, compiled_library);
		}
		else
		{
			utl::vector<utl::vector<std::string>> cached_permutations{};
			for (const auto& permutation : compile.permutations)
				cached_permutations.emplace_back(permutation.definitions.begin(), permutation.definitions.end());

			compilePermutations(parent_path, compile.name, shader_desc.sources, cached_permutations, compiled_library);
		}
	}
}

void CLibraryCompiler::compilePermutations(const std::filesystem::path parent_path, const std::string& name, const utl::vector<FShaderSource>& sources, const utl::vector<utl::vector<std::string>>& cached_permutations, FCompiledLibrary& compiled_library)
{
	u64 shader_index{ 0ull };
	// Process all shader included sources
	for (auto& source : sources)
	{
		// Generate shader code (apply extensions, process includes and definitions)
		auto base_shader_code = m_pPreprocessor->generate(parent_path, source);
		auto name_type_hash = get_shader_type(source.type);
		auto& shader_pool = compiled_library.shaders[name_type_hash];

		// Process and compile all definition combinations
//#ifdef NDEBUG
//		std::for_each(std::execution::par, cached_permutations.begin(), cached_permutations.end(),
//#else
		std::for_each(cached_permutations.begin(), cached_permutations.end(),
//#endif
			[&](const utl::vector<std::string>& definitions)
			{
				bool is_compiling_first_time{ false };
				FCompileInfo compile_info{ source.type, source.shader_name };

				// This thing will add unique definitions by shader type. Dor example for vertex shader it will add VERTEX_SHADER definition
				utl::vector<std::string> unique_shader_definitions(definitions.begin(), definitions.end());
				unique_shader_definitions.emplace_back(get_definition_by_shader_type(source.type));

				// Make unique shader name(key)
				auto unique_module_name = make_module_unique_name_str(name, definitions);

				// Get shader module
				m_locks[0].lock();
				utl::vector<FShaderModule>& shader_modules = compiled_library.modules[unique_module_name];
				m_locks[0].unlock();

				// Apply definitions for current permutation
				std::stringstream ss{ "" };
				for (auto& definition : unique_shader_definitions)
					ss << std::format("#define {}", definition) << std::endl;
				ss << std::endl;

				auto shader_code = emplace_data(base_shader_code, "{other_definitions}", ss.str());

				// Preprocess generated code (and remove ugly code after preprocessing)
				std::string preprocessed_code{};
				u32 preprocessed_hash{ 0ull };
				m_pShaderCompiler->preprocess(shader_code, compile_info, preprocessed_code, preprocessed_hash);

				if (preprocessed_code.empty())
				{
					log_error("Something went wrong while preprocessing code for module: \"{}\". Empty string after preprocessing.", unique_module_name);
					return; // throw error here
				}

				// Make hash for preprocessed data
				m_locks[1].lock();
				auto& compiled_spirv = shader_pool[preprocessed_hash];
				m_locks[1].unlock();

				m_locks[2].lock();
				if (shader_modules.empty() || (shader_modules.size() <= shader_index))
				{
					shader_modules.emplace_back(source.type, 0ull);
					is_compiling_first_time = true;
				}
				m_locks[2].unlock();

				auto& current_module = shader_modules.at(shader_index);

				bool is_code_outdated = current_module.link != utl::cast<int64_t>(preprocessed_hash);
				bool is_type_missmatch = current_module.type != source.type;

				// Notify, that type was changed
				if (is_type_missmatch)
				{
					m_locks[3].lock();
					log_warning("Type mismatch for shader module named: \"{}\". Previous type: \"{}\" new type: \"{}\". Old shaders will be removed.",
						unique_module_name, get_shader_type_str(current_module.type), get_shader_type_str(source.type));
					m_locks[3].unlock();

					// Removing old shaders
					m_locks[1].lock();
					for (auto& shmodule : shader_modules)
						shader_pool.erase(shmodule.link);
					m_locks[1].unlock();

					// Cleaning links
					m_locks[2].lock();
					shader_modules.clear();
					current_module = shader_modules.emplace_back(source.type, 0ull);
					m_locks[2].unlock();
				}

				// Compile shader if we need
				if (is_code_outdated || is_type_missmatch || is_compiling_first_time)
				{
					compiled_spirv = m_pShaderCompiler->compile(preprocessed_code, compile_info);

					std::stringstream ss{};
					ss << source.shader_name;
					for (auto& definition : definitions)
						ss << ":" << definition;

					m_locks[3].lock();
					log_info("{} shader {}[{}] was {}.", name_type_hash, ss.str(), preprocessed_hash, is_compiling_first_time ? "compiled" : "recompiled");
					m_locks[3].unlock();
				}

				// Removing old shader data
				m_locks[1].lock();
				if ((is_code_outdated || is_type_missmatch) && !is_compiling_first_time)
					shader_pool.erase(current_module.link);
				m_locks[1].unlock();

				current_module.link = utl::cast<int64_t>(preprocessed_hash);
			}
		);

		shader_index++;
	}
}