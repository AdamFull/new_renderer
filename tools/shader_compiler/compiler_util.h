#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <udetail.hpp>
#include "compiler_structures.h"

namespace shader_compiler
{
	constexpr const uint64_t MOD = static_cast<uint64_t>(-1);

	inline std::string make_shader_hashname(const std::vector<std::string>& tokens)
	{
		uint64_t result_hash{ 0ull };
		for (auto& token : tokens)
			result_hash = (result_hash + utl::fnv1a_64_hash(token.c_str())) % MOD;

		std::stringstream ss;
		ss << std::hex << result_hash;
		return ss.str();
	}

	inline size_t get_shader_type(EShaderType type)
	{
		switch (type)
		{
		case EShaderType::eUnknown: return 0ull; break;
		case EShaderType::eVertex: return utl::fnv1a_64_hash("vertex"); break;
		case EShaderType::eControl: return utl::fnv1a_64_hash("control"); break;
		case EShaderType::eEvaluation: return utl::fnv1a_64_hash("evaluation"); break;
		case EShaderType::eGeometry: return utl::fnv1a_64_hash("geometry"); break;
		case EShaderType::eFragment: return utl::fnv1a_64_hash("fragment"); break;
		case EShaderType::eCompute: return utl::fnv1a_64_hash("compute"); break;
		case EShaderType::eRayGen: return utl::fnv1a_64_hash("raygen"); break;
		case EShaderType::eIntersection: return utl::fnv1a_64_hash("intersection"); break;
		case EShaderType::eAnyHit: return utl::fnv1a_64_hash("anyhit"); break;
		case EShaderType::eClosestHit: return utl::fnv1a_64_hash("closesthit"); break;
		case EShaderType::eMiss: return utl::fnv1a_64_hash("miss"); break;
		case EShaderType::eCallable: return utl::fnv1a_64_hash("callable"); break;
		case EShaderType::eTask: return utl::fnv1a_64_hash("task"); break;
		case EShaderType::eMesh: return utl::fnv1a_64_hash("mesh"); break;
		}

		return 0ull;
	}

	inline size_t make_module_unique_name(const std::string& shader_name, const std::vector<std::string>& definitions)
	{
		std::vector<std::string> shader_name_tokens{ shader_name };
		shader_name_tokens.insert(shader_name_tokens.end(), definitions.begin(), definitions.end());
		return utl::fnv1a_64_hash(make_shader_hashname(shader_name_tokens).c_str());
	}

	inline std::string make_shader_unique_name(size_t value)
	{
		std::stringstream stream;
		stream << std::hex << value;
		return stream.str();
	}
}