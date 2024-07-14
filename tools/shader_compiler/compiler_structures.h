#pragma once

#include <string>
#include <vector>
#include <container/hash_map.h>

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
		utl::vector<std::string> definitions;
	};

	// Definition combine rule description
	struct FDefinitionVariant
	{
		utl::vector<std::string> definitions;
		EPermutationVariantCombineType type;
	};

	// Rule describes how to generate permutation
	struct FPermutationRule
	{
		utl::vector<std::string> base;
		utl::vector<FDefinitionVariant> variants;
	};

	// Describes how to generate shader permutations
	struct FShaderPermutationGenerator
	{
		utl::vector<FPermutationRule> rules;
	};

	// Pattern for generate permutation
	struct FPermutationPattern
	{
		FShaderPermutationGenerator generator;
		utl::vector<FDefinitionList> permutations;
	};

	struct FShaderMakeInfo
	{
		EShaderLang lang;
		ETargetAPIType targetApi;

		utl::hash_map<std::string, FPermutationPattern> permutationPatterns;

		utl::vector<std::string> includeDirectories;
		utl::vector<std::string> commonDirectories;

		utl::vector<std::string> buildModules;
	};

	struct FShaderMeta
	{
		i32 version{ 450 };

		utl::vector<std::string> enable_extensions;
		utl::vector<std::string> require_extensions;
		utl::vector<std::string> definitions;

		EShaderType type{ EShaderType::eUnknown };
		std::string source;
	};

	// Description about HOW to compile shader
	struct FShaderCompile
	{
		std::string name;
		std::string pattern; // Build pattern with permutations
		utl::vector<FDefinitionList> permutations;
	};

	// Shader sources description (used sources)
	struct FShaderDescription
	{
		std::string name;
		utl::vector<std::string> sources;
	};

	// Describes shader library
	struct FShaderLibrary
	{
		std::string name;
		utl::hash_map<std::string, FShaderDescription> shaders;
		utl::vector<FShaderCompile> compiles;
	};
}