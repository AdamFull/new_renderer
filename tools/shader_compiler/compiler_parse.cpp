#include "compiler_parse.h"

namespace shader_compiler
{
	void deserialize(const sjson::sjson_object& sjson, ETargetAPIType& value)
	{
		std::string temp{};
		sjson.parse_to(temp);

		if (temp == "vk10")
			value = ETargetAPIType::eVK10;
		else if (temp == "vk11")
			value = ETargetAPIType::eVK11;
		else if (temp == "vk12")
			value = ETargetAPIType::eVK12;
		else if (temp == "vk13")
			value = ETargetAPIType::eVK13;
		else if (temp == "gl")
			value = ETargetAPIType::eGL;
		else if (temp == "dx11")
			value = ETargetAPIType::eDX11;
		else if (temp == "dx12")
			value = ETargetAPIType::eDX12;
		else if (temp == "mtl")
			value = ETargetAPIType::eMTL;
		else
			value = ETargetAPIType::eUnknown;
	}

	void deserialize(const sjson::sjson_object& sjson, EShaderLang& value)
	{
		std::string temp{};
		sjson.parse_to(temp);

		if (temp == "glsl")
			value = EShaderLang::eGLSL;
		else if (temp == "slang")
			value = EShaderLang::eSLang;
		else
			value = EShaderLang::eUnknown;
	}

	void deserialize(const sjson::sjson_object& sjson, EPermutationVariantCombineType& value)
	{
		std::string temp{};
		sjson.parse_to(temp);

		if (temp == "or")
			value = EPermutationVariantCombineType::eOR;
		else if (temp == "and")
			value = EPermutationVariantCombineType::eAND;
	}

	void deserialize(const sjson::sjson_object& sjson, EShaderType& value)
	{
		std::string temp{};
		sjson.parse_to(temp);

		if (temp == "vertex")
			value = EShaderType::eVertex;
		else if (temp == "control")
			value = EShaderType::eControl;
		else if (temp == "evaluation")
			value = EShaderType::eEvaluation;
		else if (temp == "geometry")
			value = EShaderType::eGeometry;
		else if (temp == "fragment")
			value = EShaderType::eFragment;
		else if (temp == "compute")
			value = EShaderType::eCompute;
		else if (temp == "raygen")
			value = EShaderType::eRayGen;
		else if (temp == "intersection")
			value = EShaderType::eIntersection;
		else if (temp == "anyhit")
			value = EShaderType::eAnyHit;
		else if (temp == "closesthit")
			value = EShaderType::eClosestHit;
		else if (temp == "miss")
			value = EShaderType::eMiss;
		else if (temp == "callable")
			value = EShaderType::eCallable;
		else if (temp == "task")
			value = EShaderType::eTask;
		else if (temp == "mesh")
			value = EShaderType::eMesh;
		else
			value = EShaderType::eUnknown;
	}

	void deserialize(const sjson::sjson_object& sjson, FDefinitionList& value)
	{
		sjson.try_parse_to("defines", value.definitions, true);
	}

	void deserialize(const sjson::sjson_object& sjson, FDefinitionVariant& value)
	{
		sjson.try_parse_to("defines", value.definitions, true);
		sjson.try_parse_to("type", value.type, true);
	}

	void deserialize(const sjson::sjson_object& sjson, FPermutationRule& value)
	{
		sjson.try_parse_to("base", value.base, true);
		sjson.try_parse_to("variants", value.variants, true);
	}

	void deserialize(const sjson::sjson_object& sjson, FShaderPermutationGenerator& value)
	{
		sjson.try_parse_to("rules", value.rules, true);
	}

	void deserialize(const sjson::sjson_object& sjson, FPermutationPattern& value)
	{
		sjson.try_parse_to("permutation_generator", value.generator);
		sjson.try_parse_to("permutations", value.permutations);
	}

	void deserialize(const sjson::sjson_object& sjson, FShaderMakeInfo& value)
	{
		sjson.try_parse_to("lang", value.lang, true);
		sjson.try_parse_to("target_api", value.targetApi, true);

		sjson.try_parse_to("permutation_patterns", value.permutationPatterns);

		sjson.try_parse_to("include_directories", value.includeDirectories);
		sjson.try_parse_to("common_directories", value.commonDirectories);

		sjson.try_parse_to("build_modules", value.buildModules);
	}

	void deserialize(const sjson::sjson_object& sjson, FShaderMeta& value)
	{
		sjson.try_parse_to("definitions", value.definitions);

		sjson.try_parse_to("type", value.type, true);
		sjson.try_parse_to("source", value.source, true);
	}


	void deserialize(const sjson::sjson_object& sjson, FShaderDescription& value)
	{
		sjson.try_parse_to("name", value.name, true);
		sjson.try_parse_to("sources", value.sources, true);
	}

	void deserialize(const sjson::sjson_object& sjson, FShaderCompile& value)
	{
		sjson.try_parse_to("shader_name", value.name, true);
		sjson.try_parse_to("pattern", value.pattern);
		sjson.try_parse_to("permutations", value.permutations);
	}


	void deserialize(const sjson::sjson_object& sjson, FShaderLibrary& value)
	{
		sjson.try_parse_to("name", value.name, true);

		utl::vector<FShaderDescription> descriptions;
		sjson.try_parse_to("shaders", descriptions);

		for (auto& description : descriptions)
			value.shaders.emplace(description.name, description);

		sjson.try_parse_to("compile", value.compiles);
	}
}