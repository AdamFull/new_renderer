#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <regex>

#include <logger/logger.h>

#include "library_compiler.h"
#include "shader_preprocessor.h"

#include "compiler_parse.h"
#include "shader_library_parse.hpp"
#include "compiler_util.h"

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

class CArgumentParser
{
public:
	CArgumentParser(int& argc, char** argv)
	{
		for (int i = 1; i < argc; ++i)
			this->tokens.push_back(std::string(argv[i]));
	}

	template<class _Ty>
	_Ty try_get(const std::string& option, _Ty default_value)
	{
		if (!exists(option))
			return default_value;

		auto svalue = get(option).value();

		if constexpr (std::is_same_v<_Ty, std::string>)
			return svalue;

		_Ty value;
		std::stringstream ss;
		ss << svalue;
		ss >> value;

		return value;
	}

	std::optional<std::string> get(const std::string& option) const
	{
		std::vector<std::string>::const_iterator itr;
		itr = std::find(this->tokens.begin(), this->tokens.end(), option);
		if (itr != this->tokens.end() && ++itr != this->tokens.end())
			return *itr;
		return std::nullopt;
	}

	bool exists(const std::string& option) const
	{
		return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
	}
private:
	std::vector<std::string> tokens;
};

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
	log_add_file_output("shader_compiler.log");
	log_add_cout_output();
	log_init("shader_compiler", "1.0.0");

	//std::string code = R"(
	//	template<class T>
	//	T MyFunc(in T o)
	//	{
	//		T value = static_cast<T>(0);
	//		return value;
	//	}
	//
	//	template<>
	//	uint32_t MyFunc(in uint32_t o)
	//	{
	//		return static_cast<int32_t>(o) - 1;
	//	}
	//
	//	MyFunc<uint32_t>(2u);
	//	MyFunc<int32_t>(2);
	//	)";
	//
	//CShaderPreprocessor preprocessor{};
	//preprocessor.preprocess(code);

	CArgumentParser argparse(argc, argv);

	fs::path input_path{};
	fs::path compiled_libraries_output{};

	input_path = "D:\\GitHub\\new_renderer\\embed\\shaders\\config.smake";
	compiled_libraries_output = "D:\\GitHub\\new_renderer\\bin\\Debug\\engine\\embed\\rendering\\shader_cache.shc";

	FShaderMakeInfo make_info{};
	if (!parse_file_to(input_path, make_info))
	{
		log_error("Failed to open shader make file.");
		return -1;
	}

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

	auto make_file_dir = input_path.parent_path();
	fs::path shaders_common = make_file_dir / make_info.commonDirectories.front();
	fs::path shaders_include = make_file_dir / make_info.includeDirectories.front();

	// Initialize library compiler
	auto library_compiler = std::make_unique<CLibraryCompiler>(shaders_common, shaders_include);
	library_compiler->initialize(make_info.targetApi, make_info.lang, build_type);

	FShaderCacheData shader_cahce{};

	if (!parse_json_to(compiled_libraries_output, shader_cahce))
		log_info("No shader cache found.");

	// Compile all modules
	for (const auto& module_local_path : make_info.buildModules)
	{
		log_info("Building module: {}", module_local_path);
		fs::path module_path = make_file_dir / module_local_path;
		library_compiler->compile(module_path, shader_cahce);
	}

	dump_json_from(compiled_libraries_output, shader_cahce);

	log_info("Cache saved to: {}", compiled_libraries_output.string());

	return 0;
}