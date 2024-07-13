#pragma once

#include <parsing/sjson_parser.h>
#include "compiler_structures.h"

namespace shader_compiler
{
	void deserialize(const sjson::sjson_object& sjson, ETargetAPIType& value);
	void deserialize(const sjson::sjson_object& sjson, EShaderLang& value);

	void deserialize(const sjson::sjson_object& sjson, EPermutationVariantCombineType& value);
	void deserialize(const sjson::sjson_object& sjson, FDefinitionList& value);
	void deserialize(const sjson::sjson_object& sjson, FDefinitionVariant& value);
	void deserialize(const sjson::sjson_object& sjson, FPermutationRule& value);
	void deserialize(const sjson::sjson_object& sjson, FShaderPermutationGenerator& value);
	void deserialize(const sjson::sjson_object& sjson, FPermutationPattern& value);

	void deserialize(const sjson::sjson_object& sjson, FShaderMakeInfo& value);
	void deserialize(const sjson::sjson_object& sjson, FShaderMeta& value);
	void deserialize(const sjson::sjson_object& sjson, FShaderDescription& value);
	void deserialize(const sjson::sjson_object& sjson, FShaderCompile& value);
	void deserialize(const sjson::sjson_object& sjson, FShaderLibrary& value);
}