#pragma once

#include <regex>
#include <string>
#include <vector>
#include <unordered_map>

namespace shader_compiler
{
	struct FTemplateArgumentType
	{
		std::string name;
		std::string defaultVal;
	};

	struct FArgumentType
	{
		std::string name;
		std::string type;
		std::string defaultVal;
		std::string modifier;
	};

	struct FTemplateFunction
	{
		std::string name;
		std::vector<FTemplateArgumentType> templateArguments;
		std::string returnType;
		std::vector<FArgumentType> inputArguments;
		std::string code;
	};

	struct FTemplateFunctionCall
	{
		std::string name; // function name
		std::vector<std::string> templateArguments; // Should be parsed from <> or by input type
		std::vector<std::string> inputArguments;
	};

	class CShaderPreprocessor
	{
	public:
		std::string preprocess(const std::string& code);
	protected:
		std::vector<FTemplateFunction> parseTemplateFunctions(const std::string& code);
		std::vector<FTemplateFunctionCall> parseTemplateFunctionCalls(const std::string& code);
		std::unordered_map<std::string, std::string> generateTemplateFunctionCode(const std::vector<FTemplateFunction>& functions, std::vector<FTemplateFunctionCall>& calls);
	private:
		std::regex m_templateFuncRegex{ R"(template<(.+?)>\s*(\w+)\s+(\w+)\((.*?)\)\s*\{([\s\S]*?)\})" };
		std::regex m_templateArgumentRegex{ R"((?:typename|class)\s+(\w+)(?:\s*=\s*(\w+))?)" };
		std::regex m_functionArgumentRegex{ R"(([\w\s]*?)\s+([\w\s]+?)\s+(\w+)(?:\s*=\s*(\w+))?(?:\s*([\w\s]+))?)" };

		std::regex m_templateFunctionCallRegex{ R"((\w+)\s*<\s*(\w+)\s*>\s*\((.*?)\))", std::regex_constants::ECMAScript };
	};
}