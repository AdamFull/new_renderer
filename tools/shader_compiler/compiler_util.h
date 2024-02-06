#pragma once

#include <sstream>
#include <filesystem>

#include <nlohmann/json.hpp>
#include <utils/hash.h>
#include "compiler_structures.h"
#include <parsing/sjson_parser.h>

#include <logger/logger.h>

namespace shader_compiler
{
	constexpr const uint64_t MOD = static_cast<uint64_t>(-1);

	inline std::string make_shader_hashname(const std::vector<std::string>& tokens)
	{
		uint64_t result_hash{ 0ull };
		for (auto& token : tokens)
			result_hash = (result_hash + utl::utils::fnv1a_64_hash(token.c_str())) % MOD;

		std::stringstream ss;
		ss << std::hex << result_hash;
		return ss.str();
	}

	inline size_t get_shader_type(EShaderType type)
	{
		switch (type)
		{
		case EShaderType::eUnknown: return 0ull; break;
		case EShaderType::eVertex: return utl::utils::fnv1a_64_hash("vertex"); break;
		case EShaderType::eControl: return utl::utils::fnv1a_64_hash("control"); break;
		case EShaderType::eEvaluation: return utl::utils::fnv1a_64_hash("evaluation"); break;
		case EShaderType::eGeometry: return utl::utils::fnv1a_64_hash("geometry"); break;
		case EShaderType::eFragment: return utl::utils::fnv1a_64_hash("fragment"); break;
		case EShaderType::eCompute: return utl::utils::fnv1a_64_hash("compute"); break;
		case EShaderType::eRayGen: return utl::utils::fnv1a_64_hash("raygen"); break;
		case EShaderType::eIntersection: return utl::utils::fnv1a_64_hash("intersection"); break;
		case EShaderType::eAnyHit: return utl::utils::fnv1a_64_hash("anyhit"); break;
		case EShaderType::eClosestHit: return utl::utils::fnv1a_64_hash("closesthit"); break;
		case EShaderType::eMiss: return utl::utils::fnv1a_64_hash("miss"); break;
		case EShaderType::eCallable: return utl::utils::fnv1a_64_hash("callable"); break;
		case EShaderType::eTask: return utl::utils::fnv1a_64_hash("task"); break;
		case EShaderType::eMesh: return utl::utils::fnv1a_64_hash("mesh"); break;
		}

		return 0ull;
	}

	inline std::string get_shader_type_str(EShaderType type)
	{
		switch (type)
		{
		case EShaderType::eUnknown: return "invalid"; break;
		case EShaderType::eVertex: return "vertex"; break;
		case EShaderType::eControl: return "control"; break;
		case EShaderType::eEvaluation: return "evaluation"; break;
		case EShaderType::eGeometry: return "geometry"; break;
		case EShaderType::eFragment: return "fragment"; break;
		case EShaderType::eCompute: return "compute"; break;
		case EShaderType::eRayGen: return "raygen"; break;
		case EShaderType::eIntersection: return "intersection"; break;
		case EShaderType::eAnyHit: return "anyhit"; break;
		case EShaderType::eClosestHit: return "closesthit"; break;
		case EShaderType::eMiss: return "miss"; break;
		case EShaderType::eCallable: return "callable"; break;
		case EShaderType::eTask: return "task"; break;
		case EShaderType::eMesh: return "mesh"; break;
		}

		return "";
	}

	inline std::string make_module_unique_name_str(const std::string& shader_name, const std::vector<std::string>& definitions)
	{
		std::vector<std::string> shader_name_tokens{ shader_name };
		shader_name_tokens.insert(shader_name_tokens.end(), definitions.begin(), definitions.end());
		return make_shader_hashname(shader_name_tokens);
	}

	inline size_t make_module_unique_name(const std::string& shader_name, const std::vector<std::string>& definitions)
	{
		return utl::utils::fnv1a_64_hash(make_module_unique_name_str(shader_name, definitions).c_str());
	}

	inline std::string make_shader_unique_name(size_t value)
	{
		std::stringstream stream;
		stream << std::hex << value;
		return stream.str();
	}

	inline bool read_file(const std::filesystem::path& file_path, std::vector<char>& buffer)
	{
		if (!std::filesystem::exists(file_path))
			return false;

		auto path_str = file_path.string();
		FILE* file = fopen(path_str.c_str(), "rb");
		if (!file)
			return false;

		fseek(file, 0, SEEK_END);
		long file_size = ftell(file);
		rewind(file);

		if (file_size == 0)
		{
			fclose(file);
			return false;
		}

		buffer.resize(file_size);
		size_t result = fread(buffer.data(), 1, file_size, file);
		fclose(file);

		if (result != file_size)
			return false;

		return true;
	}

	template<class _Ty>
	inline bool parse_file_to(const std::filesystem::path& file_path, _Ty& output)
	{
		std::vector<char> buffer{};
		if (!read_file(file_path, buffer))
			return false;

		sjson::parse(buffer).parse_to<_Ty>(output);

		return true;
	}

	template<class _Ty>
	inline bool parse_json_to(const std::filesystem::path& file_path, _Ty& output)
	{
		std::vector<char> buffer{};
		if (!read_file(file_path, buffer))
			return false;

		nlohmann::json::from_bson(buffer).get_to(output);

		return true;
	}

	inline bool parse_header(const std::filesystem::path& file_path, std::string& data)
	{
		std::vector<char> buffer{};
		if (!read_file(file_path, buffer))
			return false;

		data = std::string(buffer.begin(), buffer.end());
		return true;
	}

	template<class _Ty>
	inline bool dump_json_from(const std::filesystem::path& file_path, const _Ty& input)
	{
		auto path_str = file_path.string();
		FILE* file = fopen(path_str.c_str(), "wb");
		if (!file)
			return false;

		std::vector<uint8_t> data = nlohmann::json::to_bson(nlohmann::json(input));

		size_t result = fwrite(data.data(), 1, data.size(), file);
		fclose(file);

		if (result != data.size())
			return false;

		return true;
	}
}