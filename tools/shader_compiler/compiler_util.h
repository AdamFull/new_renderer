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
	constexpr const u32 MOD = static_cast<u32>(-1);

	inline u32 make_shader_hashname(const utl::vector<std::string>& tokens)
	{
		u32 result_hash{ 0ll };
		for (auto& token : tokens)
			result_hash = (result_hash + utl::fnv1a_32_hash_cstr(token.c_str())) % MOD;
		return result_hash;
	}

	inline u32 get_shader_type(EShaderType type)
	{
		switch (type)
		{
		case EShaderType::eUnknown: return 0ull; break;
		case EShaderType::eVertex: return utl::fnv1a_32_hash_cstr("vertex"); break;
		case EShaderType::eControl: return utl::fnv1a_32_hash_cstr("control"); break;
		case EShaderType::eEvaluation: return utl::fnv1a_32_hash_cstr("evaluation"); break;
		case EShaderType::eGeometry: return utl::fnv1a_32_hash_cstr("geometry"); break;
		case EShaderType::eFragment: return utl::fnv1a_32_hash_cstr("fragment"); break;
		case EShaderType::eCompute: return utl::fnv1a_32_hash_cstr("compute"); break;
		case EShaderType::eRayGen: return utl::fnv1a_32_hash_cstr("raygen"); break;
		case EShaderType::eIntersection: return utl::fnv1a_32_hash_cstr("intersection"); break;
		case EShaderType::eAnyHit: return utl::fnv1a_32_hash_cstr("anyhit"); break;
		case EShaderType::eClosestHit: return utl::fnv1a_32_hash_cstr("closesthit"); break;
		case EShaderType::eMiss: return utl::fnv1a_32_hash_cstr("miss"); break;
		case EShaderType::eCallable: return utl::fnv1a_32_hash_cstr("callable"); break;
		case EShaderType::eTask: return utl::fnv1a_32_hash_cstr("task"); break;
		case EShaderType::eMesh: return utl::fnv1a_32_hash_cstr("mesh"); break;
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

	inline u32 make_module_unique_name_str(const std::string& shader_name, const utl::vector<std::string>& definitions)
	{
		utl::vector<std::string> shader_name_tokens{ shader_name };
		shader_name_tokens.insert(shader_name_tokens.end(), definitions.begin(), definitions.end());
		return make_shader_hashname(shader_name_tokens);
	}

	inline u32 make_module_unique_name(const std::string& shader_name, const utl::vector<std::string>& definitions)
	{
		return make_module_unique_name_str(shader_name, definitions);
	}

	inline std::string make_shader_unique_name(size_t value)
	{
		std::stringstream stream;
		stream << std::hex << value;
		return stream.str();
	}

	template<typename _Ty>
	inline bool read_file(const std::filesystem::path& file_path, utl::vector<_Ty>& buffer, const char* mode = "rb")
	{
		if (!std::filesystem::exists(file_path))
			return false;

		auto path_str = file_path.string();
		FILE* file = fopen(path_str.c_str(), mode);
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

		return true;
	}

	template<typename _Ty>
	inline bool write_file(const std::filesystem::path& file_path, utl::vector<_Ty>& buffer, const char* mode = "wb")
	{
		auto path_str = file_path.string();
		FILE* file = fopen(path_str.c_str(), mode);
		if (!file)
			return false;

		size_t result = fwrite(buffer.data(), sizeof(_Ty), buffer.size(), file);
		if (result != buffer.size())
			return false;

		fclose(file);

		return true;
	}

	template<class _Ty>
	inline bool parse_file_to(const std::filesystem::path& file_path, _Ty& output)
	{
		utl::vector<uint8_t> buffer{};
		if (!read_file(file_path, buffer))
			return false;

		sjson::parse(buffer).parse_to<_Ty>(output);

		return true;
	}

	template<class _Ty>
	inline bool parse_json_to(const std::filesystem::path& file_path, _Ty& output)
	{
		utl::vector<uint8_t> buffer{};
		if (!read_file(file_path, buffer))
			return false;

		nlohmann::json::from_bson(buffer.begin(), buffer.end()).get_to(output);

		return true;
	}

	inline bool parse_header(const std::filesystem::path& file_path, std::string& data)
	{
		utl::vector<uint8_t> buffer{};
		if (!read_file(file_path, buffer))
			return false;

		data = std::string(buffer.begin(), buffer.end());
		return true;
	}

	template<class _Ty>
	inline bool dump_json_from(const std::filesystem::path& file_path, const _Ty& input, const char* mode = "wb")
	{
		std::vector<uint8_t> data = nlohmann::json::to_bson(nlohmann::json(input));
		if (write_file(file_path, data, mode))
			return true;

		return false;
	}

	inline void make_shader_compiler_fatal_error(const std::string& name, std::span<uint8_t> code, const std::string& error)
	{
		auto current_path = std::filesystem::current_path();
		log_error(error);

		auto output_directory = current_path / "error_dumps";
		if (!std::filesystem::exists(output_directory))
			std::filesystem::create_directories(output_directory);

		auto path_string = (output_directory / (name + ".glsl")).string();

		FILE* file = fopen(path_string.c_str(), "wb");
		if (!file)
			return;

		fwrite(error.data(), 1, error.size(), file);
		fwrite(code.data(), 1, code.size(), file);
		fclose(file);

		log_info("Shader file was saved in: {}", path_string);
		terminate();
	}
}