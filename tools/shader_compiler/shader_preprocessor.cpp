#include "shader_preprocessor.h"

#include <format>

using namespace shader_compiler;

// TODO: 
// Add validation
// Add support for default template type (for example template<uint32_t _count>)
// Add template function specialization support
// Add template support for struct

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