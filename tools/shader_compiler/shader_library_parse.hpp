#pragma once

#include <parsing/json_parser.h>
#include "compiler_structures.h"

namespace shader_compiler
{
    NLOHMANN_JSON_SERIALIZE_ENUM(EShaderType,
        {
            { EShaderType::eUnknown, nullptr },
            { EShaderType::eVertex, "vertex" },
            { EShaderType::eControl, "control" },
            { EShaderType::eEvaluation, "evaluation" },
            { EShaderType::eGeometry, "geometry" },
            { EShaderType::eFragment, "fragment" },
            { EShaderType::eCompute, "compute" },
            { EShaderType::eRayGen, "raygen" },
            { EShaderType::eIntersection, "intersection" },
            { EShaderType::eAnyHit, "anyhit" },
            { EShaderType::eClosestHit, "closesthit" },
            { EShaderType::eMiss, "miss" },
            { EShaderType::eCallable, "callable" },
            { EShaderType::eTask, "task" },
            { EShaderType::eMesh, "mesh" },
        })

    // Contain info about shader type and links to code
    struct FShaderModule
    {
        EShaderType type{ EShaderType::eUnknown };
        int64_t link{ 0ll };
    };

    // Contains associative maps for cantaining compiled shader code
    struct FCompiledLibrary
    {
        /*
        * shaders:
        *   vertex:
        *       "shader_hash": "code"
        *
        * modules:
        *   module_unique_name: [
        *       { type, shader_hash }
        * ]
        */

        // Key is the type of shader (EShaderType) it's like container of same shader types. 
        // Key in value it's link to shader from shader module. 
        std::unordered_map<int64_t, std::unordered_map<int64_t, std::vector<u32>>> shaders;
        std::unordered_map<int64_t, std::vector<FShaderModule>> modules;
    };

    struct FShaderCacheData
    {
        std::unordered_map<int64_t, FCompiledLibrary> libraries{};
    };

    inline void to_json(nlohmann::json& json, const FShaderModule& type)
    {
        json = nlohmann::json();
        utl::parsing::serialize_from("type", json, type.type, true);
        utl::parsing::serialize_from("link", json, type.link, true);
    }

    inline void from_json(const nlohmann::json& json, FShaderModule& type)
    {
        utl::parsing::parse_to("type", json, type.type);
        utl::parsing::parse_to("link", json, type.link);
    }


    inline void to_json(nlohmann::json& json, const FCompiledLibrary& type)
    {
        json = nlohmann::json();
        utl::parsing::serialize_from("shaders", json, type.shaders, true);
        utl::parsing::serialize_from("modules", json, type.modules, true);
    }

    inline void from_json(const nlohmann::json& json, FCompiledLibrary& type)
    {
        utl::parsing::parse_to("shaders", json, type.shaders);
        utl::parsing::parse_to("modules", json, type.modules);
    }


    inline void to_json(nlohmann::json& json, const FShaderCacheData& type)
    {
        json = nlohmann::json();
        utl::parsing::serialize_from("libraries", json, type.libraries, true);
    }

    inline void from_json(const nlohmann::json& json, FShaderCacheData& type)
    {
        utl::parsing::parse_to("libraries", json, type.libraries);
    }
}

namespace nlohmann
{
    template <typename _Ty>
    struct adl_serializer<std::unordered_map<int64_t, _Ty>>
    {
        static void to_json(json& j, const std::unordered_map<int64_t, _Ty>& map)
        {
            for (const auto& [key, value] : map)
                j[std::to_string(key)] = value;
        }

        static void from_json(const json& j, std::unordered_map<int64_t, _Ty>& map)
        {
            for (const auto& item : j.items())
            {
                int64_t key = std::stoll(item.key());
                map[key] = item.value().get<_Ty>();
            }
        }
    };
}