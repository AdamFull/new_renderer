#include "shader_preprocessor.h"
#include "compiler_parse.h"

#include "compiler_util.h"

#include <format>

using namespace shader_compiler;

// TODO: 
// Add validation
// Add support for default template type (for example template<uint32_t _count>)
// Add template function specialization support
// Add template support for struct

std::string process_include(const std::string& include, const std::vector<std::filesystem::path>& includes_path, EShaderLang lang, u64 depth);

std::string generate_shader_code(const FShaderObject& sobject, const std::vector < std::filesystem::path>& includes_path, EShaderLang lang, u64 depth = 0)
{
	std::stringstream ss{};

	if (depth == 0ull)
	{
		if (lang == EShaderLang::eGLSL)
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
		}

		// Apply definitions (TODO: add definition values)
		for (auto& definition : sobject.definitions)
			ss << std::format("#define {}", definition) << std::endl;

		if (depth == 0ull)
			ss << "{other_definitions}" << std::endl << std::endl; // For other definitions
	}

	// Include 
	for (auto& sinclude : sobject.includes)
		ss << process_include(sinclude, includes_path, lang, depth);

	// Apply code
	ss << sobject.code;

	return ss.str();
}

std::filesystem::path find_include_path(const std::string& include, const std::vector<std::filesystem::path>& includes_path)
{
	for (const auto& include_path : includes_path)
	{
		auto base_includes_path = (include_path / include).string();
		if (std::filesystem::exists(base_includes_path + ".shinc"))
			return std::filesystem::weakly_canonical(std::filesystem::path(base_includes_path + ".shinc"));
		else if (std::filesystem::exists(base_includes_path + ".h"))
			return std::filesystem::weakly_canonical(std::filesystem::path(base_includes_path + ".h"));
	}

	return {};
}

std::string process_include(const std::string& include, const std::vector<std::filesystem::path>& includes_path, EShaderLang lang, u64 depth)
{
	std::stringstream ss{};
	std::string guard_name{};

	auto include_file_path = find_include_path(include, includes_path);
	auto file_ext = include_file_path.extension().string();

	FShaderObject include_so{};
	if (file_ext == ".shinc" && parse_file_to(include_file_path, include_so))
		guard_name = std::format("{}_protector", include_file_path.stem().string());
	else if (file_ext == ".h" && parse_header(include_file_path, include_so.code))
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
	ss << generate_shader_code(include_so, includes_path, lang, depth + 1);
	ss << "\n#endif" << std::endl << std::endl;

	return ss.str();
}

CShaderPreprocessor::CShaderPreprocessor(const std::filesystem::path& common_directory, const std::vector<std::filesystem::path>& include_directories)
{
	m_includeDirectories = include_directories;
	m_commonDirectory = common_directory;
}

void CShaderPreprocessor::initialize(EShaderLang lang)
{
	m_shaderLang = lang;
}

std::string CShaderPreprocessor::generate(const std::filesystem::path& path, const FShaderSource& source)
{
	// TODO: i think i should create common library that will contatin common sources
	auto shader_path = (source.common ? m_commonDirectory : path) / (source.shader_name + ".shade");

	// Parse shader file
	FShaderObject shader_object{};
	if (!parse_file_to(shader_path, shader_object))
	{
		log_error("Failed to parse shader: {}.", source.shader_name);
		return {};
	}

	return generate_shader_code(shader_object, m_includeDirectories, m_shaderLang);
}

std::string CShaderPreprocessor::preprocess(const std::string& code)
{
	auto template_functions = parseTemplateFunctions(code);
	auto template_calls = parseTemplateFunctionCalls(code);

	auto generated_template_functions_code = generateTemplateFunctionCode(template_functions, template_calls);

	return "";
}

std::vector<FTemplateFunction> CShaderPreprocessor::parseTemplateFunctions(const std::string& code)
{
	std::vector<FTemplateFunction> templates;
	
	for (std::sregex_iterator i = std::sregex_iterator(code.begin(), code.end(), m_templateFuncRegex); i != std::sregex_iterator(); ++i)
	{
		std::smatch match = *i;
		if (match.size() == 6)
		{
			auto& func = templates.emplace_back();
			func.returnType = match[2].str();
			func.name = match[3].str();

			std::string templateArgsStr = match[1].str();
			std::string inputArgsStr = match[4].str();
			func.code = match[5].str();

			// Template argiments parsing
			for (std::sregex_iterator j = std::sregex_iterator(templateArgsStr.begin(), templateArgsStr.end(), m_templateArgumentRegex); j != std::sregex_iterator(); ++j)
			{
				std::smatch argMatch = *j;
				if (argMatch.size() == 3)
				{
					auto& arg = func.templateArguments.emplace_back();
					arg.name = argMatch[1].str();
					arg.defaultVal = argMatch[2].str();
				}
			}

			// Input argument parsing
			for (std::sregex_iterator k = std::sregex_iterator(inputArgsStr.begin(), inputArgsStr.end(), m_functionArgumentRegex); k != std::sregex_iterator(); ++k)
			{
				std::smatch inputArgMatch = *k;
				if (inputArgMatch.size() > 3)
				{
					auto& inputArg = func.inputArguments.emplace_back();
					inputArg.modifier = inputArgMatch[1].str().empty() ? "" : inputArgMatch[1].str();
					inputArg.type = inputArgMatch[2].str();
					inputArg.name = inputArgMatch[3].str();

					if (inputArgMatch.size() > 4 && inputArgMatch[4].matched)
						inputArg.defaultVal = inputArgMatch[4].str();
				}
			}
		}
	}

	// Parse here specializations

	return templates;
}

std::vector<FTemplateFunctionCall> CShaderPreprocessor::parseTemplateFunctionCalls(const std::string& code)
{
	std::vector<FTemplateFunctionCall> calls;

	for (std::sregex_iterator i = std::sregex_iterator(code.begin(), code.end(), m_templateFunctionCallRegex); i != std::sregex_iterator(); ++i)
	{
		std::smatch match = *i;
		if (match.size() == 4)
		{
			auto& call = calls.emplace_back();
			call.name = match[1].str();
			call.templateArguments.push_back(match[2].str());

			std::regex argSplitRegex(R"(\s*,\s*)");
			std::string argsStr = match[3].str();
			std::sregex_token_iterator iter(argsStr.begin(), argsStr.end(), argSplitRegex, -1);
			std::sregex_token_iterator end;
			for (; iter != end; ++iter)
			{
				if (!iter->str().empty())
					call.inputArguments.push_back(iter->str());
			}
		}
	}

	return calls;
}

std::unordered_map<std::string, std::string> CShaderPreprocessor::generateTemplateFunctionCode(const std::vector<FTemplateFunction>& functions, std::vector<FTemplateFunctionCall>& calls)
{
	std::unordered_map<std::string, std::string> generated_functions;

	for (const auto& function_call : calls)
	{
		std::string gen_function_name = function_call.name;
		for (const auto& template_arg : function_call.templateArguments)
			gen_function_name += '_' + template_arg;

		auto found_template = std::find_if(functions.begin(), functions.end(), [&](const FTemplateFunction& func) { return func.name == function_call.name; });
		if (found_template == functions.end())
			continue;

		const auto& template_function = *found_template;

		// TODO: type count in function can be more then in call
		// Generate type associations
		std::unordered_map<std::string, std::string> type_association{};
		for (size_t type_idx = 0ull; type_idx < function_call.templateArguments.size(); ++type_idx)
			type_association[template_function.templateArguments.at(type_idx).name] = function_call.templateArguments.at(type_idx);

		std::string return_type = template_function.returnType;

		auto found_return_type = type_association.find(return_type);
		if (found_return_type != type_association.end())
			return_type = found_return_type->second;

		std::string gen_input_arguments{};
		for (auto arg_it = template_function.inputArguments.begin(); arg_it != template_function.inputArguments.end(); ++arg_it)
		{
			std::string value_type = arg_it->type;
			auto found_type = type_association.find(arg_it->type);
			if (found_type != type_association.end())
				value_type = found_type->second;

			auto argument_raw = std::format("{} {} {}", arg_it->modifier, value_type, arg_it->name);
			if (!arg_it->defaultVal.empty())
				argument_raw = std::format("{} = {}", argument_raw, arg_it->defaultVal);

			gen_input_arguments += argument_raw;

			if (arg_it + 1 != template_function.inputArguments.end())
				gen_input_arguments += ", ";
		}

		std::string function_header = std::format("{} {}({})", return_type, gen_function_name, gen_input_arguments);

		std::string generated_code = template_function.code;
		for (const auto& [arg, type] : type_association)
		{
			std::regex replace_regex(std::format("\\b{}\\b", arg));
			generated_code = std::regex_replace(generated_code, replace_regex, type);
		}

		generated_functions[gen_function_name] = std::format("{}\n{{\n{}\n}}", function_header, generated_code);
	}

	return generated_functions;
}