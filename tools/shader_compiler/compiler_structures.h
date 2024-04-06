#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace shader_compiler
{
	enum class ETargetAPIType
	{
		eUnknown,
		eVK10,
		eVK11,
		eVK12,
		eVK13,
		eGL,
		eDX11,
		eDX12,
		eMTL
	};

	enum class ETargetBuildType
	{
		eRelease,
		eDebug
	};

	enum class EShaderLang
	{
		eUnknown,
		eGLSL,
		eHLSL
	};

	enum class EShaderType
	{
		eUnknown,
		eVertex,
		eControl,
		eEvaluation,
		eGeometry,
		eFragment,
		eCompute,
		eRayGen,
		eIntersection,
		eAnyHit,
		eClosestHit,
		eMiss,
		eCallable,
		eTask,
		eMesh
	};

	enum class EPermutationVariantCombineType
	{
		eOR,
		eAND
	};

	// List of definitions
	struct FDefinitionList
	{
		std::vector<std::string> definitions;
	};

	// Definition combine rule description
	struct FDefinitionVariant
	{
		std::vector<std::string> definitions;
		EPermutationVariantCombineType type;
	};

	// Rule describes how to generate permutation
	struct FPermutationRule
	{
		std::vector<std::string> base;
		std::vector<FDefinitionVariant> variants;
	};

	// Describes how to generate shader permutations
	struct FShaderPermutationGenerator
	{
		std::vector<FPermutationRule> rules;
	};

	// Pattern for generate permutation
	struct FPermutationPattern
	{
		FShaderPermutationGenerator generator;
		std::vector<FDefinitionList> permutations;
	};

	struct FShaderMakeInfo
	{
		EShaderLang lang;
		ETargetAPIType targetApi;

		std::unordered_map<std::string, FPermutationPattern> permutationPatterns;

		std::vector<std::string> includeDirectories;
		std::vector<std::string> commonDirectories;

		std::vector<std::string> buildModules;
	};

	struct FShaderObject
	{
		uint32_t version;

		std::vector<std::string> enable_extensions;
		std::vector<std::string> require_extensions;
		std::vector<std::string> definitions;
		std::vector<std::string> includes;

		std::string code;
	};

	// Description about HOW to compile shader
	struct FShaderCompile
	{
		std::string name;
		std::string pattern; // Build pattern with permutations
		std::vector<FDefinitionList> permutations;
	};

	// Description about how to find shader source file
	struct FShaderSource
	{
		EShaderType type{ EShaderType::eUnknown };
		bool common{ false };
		std::string shader_name;
	};

	// Shader sources description (used sources)
	struct FShaderDescription
	{
		std::string name;
		std::vector<FShaderSource> sources;
	};

	// Describes shader library
	struct FShaderLibrary
	{
		std::string name;
		std::unordered_map<std::string, FShaderDescription> shaders;
		std::vector<FShaderCompile> compiles;
	};
}