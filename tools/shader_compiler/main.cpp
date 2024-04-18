#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <regex>

#include <logger/logger.h>
#include <utils/stopwatch.h>
#include <parsing/argparse.h>

#include "library_compiler.h"
#include "shader_preprocessor.h"

#include "compiler_parse.h"
#include "shader_library_parse.hpp"
#include "compiler_util.h"

static int allocator_version = mi_version();

namespace fs = std::filesystem;
using namespace shader_compiler;

ETargetBuildType parse_build_type(const std::string& build_type_str)
{
	if (build_type_str == "release")
		return ETargetBuildType::eRelease;
	else if (build_type_str == "debug")
		return ETargetBuildType::eDebug;

	return ETargetBuildType::eRelease;
}

fs::path make_current_library_output_path(const fs::path& base_path, const fs::path& base_out_path, const fs::path& current_path)
{
	fs::path out_path{};
	if (base_out_path.empty())
		out_path = current_path;

	auto relative = fs::relative(current_path, base_path);
	return fs::weakly_canonical(base_out_path / relative);
}

int main(int argc, char** argv)
{
	utl::parsing::argument_parser argparse{};
	argparse.register_arg("-in", "-input").set_help("Path to shader make file.").require_value();
	argparse.register_arg("-out", "-output").set_help("Path to output directory to save shader cache.").require_value();
	argparse.register_arg("-bt", "-build_type").set_help("Path to output directory to save shader cache.").require_value();

	argparse.parse_arguments(argc, argv);

	log_add_file_output("shader_compiler.log");
	log_add_cout_output();
	log_init("shader_compiler", "1.0.0");

	fs::path input_path{};
	fs::path compiled_libraries_output{};

	input_path = "D:\\GitHub\\new_renderer\\embed\\shaders\\config.smake";
	compiled_libraries_output = "D:\\GitHub\\new_renderer\\bin\\Debug\\engine\\embed\\rendering\\shader_cache.shc";

	input_path = argparse.try_get("-in", input_path);
	if (input_path.empty())
	{
		log_error("Input path is not set.");
		return -1;
	}

	compiled_libraries_output = argparse.try_get("-out", compiled_libraries_output);
	if (compiled_libraries_output.empty())
	{
		log_error("Output path is not set.");
		return -1;
	}

	std::string build_type_str{ "release" };
	build_type_str = argparse.try_get("-build_type", build_type_str);
	ETargetBuildType build_type = parse_build_type(build_type_str);

	FShaderMakeInfo make_info{};
	if (!parse_file_to(input_path, make_info))
	{
		log_error("Failed to open shader make file.");
		return -1;
	}

	auto make_file_dir = input_path.parent_path();
	fs::path shaders_common = make_file_dir / make_info.commonDirectories.front();

	utl::vector<std::filesystem::path> include_directories{};
	for (const auto& include_path : make_info.includeDirectories)
		include_directories.emplace_back(make_file_dir / include_path);

	// Initialize library compiler
	auto library_compiler = std::make_unique<CLibraryCompiler>(shaders_common, include_directories);
	library_compiler->initialize(make_info.targetApi, make_info.lang, build_type);
	library_compiler->generatePermutations(make_info.permutationPatterns);

	utl::utils::stopwatch stopwatch{};
	FShaderCacheData shader_cahce{};
	if (!parse_json_to(compiled_libraries_output, shader_cahce))
		log_info("No shader cache found.");

	// Compile all modules
	for (const auto& module_local_path : make_info.buildModules)
	{
		log_info("Building module: {}", module_local_path);
		fs::path module_path = make_file_dir / module_local_path;
		library_compiler->compile(module_path, shader_cahce);
			//dump_json_from(compiled_libraries_output, shader_cahce);
	}

	dump_json_from(compiled_libraries_output, shader_cahce);

	log_info("Cache saved to: {}", compiled_libraries_output.string());
	log_info("Compilation finished with: {:.2f}s", stopwatch.stop<float>());

	return 0;
}