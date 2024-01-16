#include "library_compiler.h"

#include "compiler_util.h"
#include "compiler_parse.h"
#include "shader_library_parse.hpp"

#include <fstream>
#include <regex>

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

std::vector<std::string> make_permutation(u64 mask, const std::vector<FDefinitionChain>& chains) 
{
	std::vector<std::string> permutation;

	for (u64 j = 0; j < chains.size(); ++j) 
	{
		if (mask & (1 << j)) 
		{
			permutation.emplace_back(chains[j].base);

			if (!chains[j].var.empty())
				permutation.emplace_back(chains[j].var[0].base);
		}
	}

	return permutation;
}

std::string process_include(const std::string& include, const std::filesystem::path& includes_path, u64 depth);

std::string generate_shader_code(const FShaderObject& sobject, const std::filesystem::path& includes_path, u64 depth = 0)
{
	std::stringstream ss{};

	if (depth == 0ull)
	{
		// Set language version
		ss << std::format("#version {}", sobject.version) << std::endl << std::endl;

		// Apply enabled extensions
		for (auto& extension_name : sobject.enable_extensions)
			ss << std::format("#extension {} : enable", extension_name) << std::endl;
		ss << std::endl;

		// Apply required extensions
		for (auto& extension_name : sobject.require_extensions)
			ss << std::format("#extension {} : require", extension_name) << std::endl;
		ss << std::endl;

		// Apply definitions (TODO: add definition values)
		for (auto& definition : sobject.definitions)
			ss << std::format("#define {}", definition) << std::endl;

		if (depth == 0ull)
			ss << "{other_definitions}" << std::endl << std::endl; // For other definitions
	}

	// Include 
	for (auto& sinclude : sobject.includes)
		ss << process_include(sinclude, includes_path, depth);

	// Apply code
	ss << sobject.code;

	return ss.str();
}

std::string process_include(const std::string& include, const std::filesystem::path& includes_path, u64 depth)
{
	std::stringstream ss{};
	std::string guard_name{};

	auto base_include_path = (includes_path / include).string();

	FShaderObject include_so{};
	if (parse_file_to(base_include_path + ".shinc", include_so))
		guard_name = std::format("{}_protector", std::filesystem::path(base_include_path).stem().string());
	else if (parse_header(base_include_path + ".h", include_so.code))
	{
		std::regex guard_start_pattern("#ifndef (.+)");
		std::regex guard_define_pattern("#define (.+)");
		std::regex guard_end_pattern("#endif.*");
		std::regex include_pattern("#include \"(.+)\"");
		std::smatch matches;

		std::istringstream stream(include_so.code);
		std::string line;
		
		bool in_guard_block{ false };
		bool is_guard_defined{ false };

		include_so.code.clear();
		while (std::getline(stream, line)) 
		{
			if (std::regex_search(line, matches, guard_start_pattern) && !in_guard_block)
			{
				guard_name = matches[1].str();
				in_guard_block = true;
			}
			else if (in_guard_block && !is_guard_defined && std::regex_search(line, matches, guard_define_pattern))
				is_guard_defined = guard_name == matches[1].str();
			else if (in_guard_block && is_guard_defined && std::regex_search(line, matches, guard_end_pattern) && stream.eof())
			{
				in_guard_block = false;
				is_guard_defined = false;
			}
			else
			{
				if (std::regex_search(line, matches, include_pattern)) 
					include_so.includes.emplace_back(matches[1].matched ? matches[1].str() : matches[2].str());
				else
					include_so.code += line + "\n";
			}
		}
	}
	else
		log_error("Include {} was not found, or not supported format.", include);

	ss << std::format("\n#ifndef {}\n#define {}", guard_name, guard_name) << std::endl;
	ss << generate_shader_code(include_so, includes_path, depth + 1);
	ss << "\n#endif" << std::endl << std::endl;

	return ss.str();
}

CLibraryCompiler::CLibraryCompiler(const std::filesystem::path& common_path, const std::filesystem::path& includes_path)
{
	m_srCommonPath = common_path;
	m_srIncludesPath = includes_path;
}


void CLibraryCompiler::initialize(ETargetAPIType target_api, ETargetBuildType build_type)
{
	m_pShaderCompiler = std::make_unique<CShaderCompiler>(target_api, build_type);
	m_pShaderCompiler->initialize();
}

void CLibraryCompiler::compile(const std::filesystem::path& library_path, FShaderCacheData& shader_cache)
{
	FShaderLibrary shader_library_ci{};
	if (!parse_file_to(library_path, shader_library_ci))
		return;

	FCompiledLibrary& compiled_library = shader_cache.libraries[utl::fnv1a_64_hash(shader_library_ci.name.c_str())];

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

		// Generate all shader permutations
		bool generate_permutations = !compile.permutation_gen.definitions.empty();
		u64 perm_n = compile.permutation_gen.definitions.size();
		u64 total_permutations = 1 << perm_n;

		std::vector<std::vector<std::string>> cached_permutations{};

		if (generate_permutations)
		{
			for (u64 mask = 0; mask < total_permutations; ++mask)
				cached_permutations.emplace_back(make_permutation(mask, compile.permutation_gen.definitions));
		}

		for (auto& permutation : compile.permutations)
			cached_permutations.emplace_back(permutation.definitions);

		u64 shader_index{ 0ull };
		// Process all shader included sources
		for (auto& source : shader_desc.sources)
		{
			// TODO: i think i should create common library that will contatin common sources
			auto shader_path = (source.common ? m_srCommonPath : parent_path) / (source.shader_name + ".shade");

			// Parse shader file
			FShaderObject shader_object{};
			if (!parse_file_to(shader_path, shader_object))
				continue;

			// Generate shader code (apply extensions, process includes and definitions)
			auto base_shader_code = generate_shader_code(shader_object, m_srIncludesPath);
			auto name_type_hash = get_shader_type(source.type);
			auto& shader_pool = compiled_library.shaders[name_type_hash];
			
			// Process and compile all definition combinations
			for (auto& definitions : cached_permutations)
			{
				bool is_compiling_first_time{ false };
				FCompileInfo compile_info{ source.type, source.shader_name };

				// This thing will add unique definitions by shader type. Dor example for vertex shader it will add VERTEX_SHADER definition
				std::vector<std::string> unique_shader_definitions(definitions.begin(), definitions.end());
				unique_shader_definitions.emplace_back(get_definition_by_shader_type(source.type));

				// Make unique shader name(key)
				auto module_name_str = make_module_unique_name_str(compile.name, definitions);
				auto unique_module_name = utl::fnv1a_64_hash(module_name_str.c_str());

				// Get shader module
				std::vector<FShaderModule>& shader_modules = compiled_library.modules[unique_module_name];

				// Apply definitions for current permutation
				std::stringstream ss{""};
				for (auto& definition : unique_shader_definitions)
					ss << std::format("#define {}", definition) << std::endl;
				ss << std::endl;

				auto shader_code = emplace_data(base_shader_code, "{other_definitions}", ss.str());

				// Preprocess generated code (and remove ugly code after preprocessing)
				std::string preprocessed_code{};
				u64 preprocessed_hash{ 0ull };
				m_pShaderCompiler->preprocess(shader_code, compile_info, preprocessed_code, preprocessed_hash);

				if (preprocessed_code.empty())
				{
					log_error("Something went wrong while preprocessing code for module: \"{}\". Empty string after preprocessing.", module_name_str);
					return; // throw error here
				}

				// Make hash for preprocessed data
				auto& compiled_spirv = shader_pool[preprocessed_hash];

				// Create new shader module
				if (shader_modules.empty() || (shader_modules.size() <= shader_index))
				{
					shader_modules.emplace_back(source.type, 0ull);
					is_compiling_first_time = true;
				}

				auto& current_module = shader_modules.at(shader_index);

				bool is_code_outdated = current_module.link != utl::cast<int64_t>(preprocessed_hash);
				bool is_type_missmatch = current_module.type != source.type;

				// Notify, that type was changed
				if (is_type_missmatch)
				{
					log_warning("Type mismatch for shader module named: \"{}\". Previous type: \"{}\" new type: \"{}\". Old shaders will be removed.", 
						module_name_str, get_shader_type_str(current_module.type), get_shader_type_str(source.type));

					// Removing old shaders
					for (auto& shmodule : shader_modules)
						shader_pool.erase(shmodule.link);

					// Cleaning links
					shader_modules.clear();
					current_module = shader_modules.emplace_back(source.type, 0ull);
				}

				// Compile shader if we need
				if (is_code_outdated || is_type_missmatch || is_compiling_first_time)
				{
					compiled_spirv = m_pShaderCompiler->compile(preprocessed_code, compile_info);
					log_info("{} shader {}[{}] was {}.", name_type_hash, source.shader_name, preprocessed_hash, is_compiling_first_time ? "compiled" : "recompiled");
				}

				// Removing old shader data
				if ((is_code_outdated || is_type_missmatch) && !is_compiling_first_time)
					shader_pool.erase(current_module.link);

				current_module.link = utl::cast<int64_t>(preprocessed_hash);
			}

			shader_index++;
		}
	}

	//dump_json_from(out_library_path, compiled_library);
}