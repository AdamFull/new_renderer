#pragma once

#include "compiler_structures.h"

#include <regex>
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

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
		CShaderPreprocessor(const std::filesystem::path& common_path, const std::vector<std::filesystem::path>& include_directories);

		void initialize(EShaderLang lang);

		std::string generate(const std::filesystem::path& path, const FShaderSource& shader_source);

		std::string preprocess(const std::string& code);
	protected:
		std::vector<FTemplateFunction> parseTemplateFunctions(const std::string& code);
		std::vector<FTemplateFunctionCall> parseTemplateFunctionCalls(const std::string& code);
		utl::hash_map<std::string, std::string> generateTemplateFunctionCode(const std::vector<FTemplateFunction>& functions, std::vector<FTemplateFunctionCall>& calls);
	private:
		EShaderLang m_shaderLang;
		std::vector<std::filesystem::path> m_includeDirectories;
		std::filesystem::path m_commonDirectory;

		std::regex m_templateFuncRegex{ R"(template<(.+?)>\s*(\w+)\s+(\w+)\((.*?)\)\s*\{([\s\S]*?)\})" };
		std::regex m_templateArgumentRegex{ R"((?:typename|class)\s+(\w+)(?:\s*=\s*(\w+))?)" };
		std::regex m_functionArgumentRegex{ R"(([\w\s]*?)\s+([\w\s]+?)\s+(\w+)(?:\s*=\s*(\w+))?(?:\s*([\w\s]+))?)" };

		std::regex m_templateFunctionCallRegex{ R"((\w+)\s*<\s*(\w+)\s*>\s*\((.*?)\))", std::regex_constants::ECMAScript };
	};
}