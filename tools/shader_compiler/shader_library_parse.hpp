#pragma once

#include <parsing/json_parser.h>
#include "compiler_structures.h"
#include <common/StringUtil.h>

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
        u32 link{ 0ll };
    };

    struct FShaderData
    {
        std::string path;
        utl::vector<u32> data;
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
        utl::hash_map<u32, utl::hash_map<u32, FShaderData>> shaders;
        utl::hash_map<u32, utl::vector<FShaderModule>> modules;
    };

    enum class EShaderReflectionBlockType
    {
        eUnknown,
        ePushConstant,
        eSpecializationConstant,
        eUniformBuffer, 
        eStorageBuffer, 
        eCombinedImageSampler, 
        eSampledImage, 
        eSampler, 
        eStorageImage, 
        eInputAttachment,
        eAccelerationStructure
    };

    enum class EShaderReflectionScopeType
    {
        eNone,
        eGlobal, 
        eLocal
    };

    struct FShaderReflectionBlockMember
    {
        //utl::id_string32 name;

        // TODO: type
        u32 offset;
        u32 size;
    };

    struct FShaderReflectionUniform
    {
        std::string name{};

        u32 set{ 0u };
        u32 binding{ 0u };
        u32 offset{ 0ull };
        u32 size{ 0ull };
    };

    struct FShaderReflectionBlock
    {
        u32 set{ 0u };
        u32 binding{ 0u };
        u64 size{ 0ull };

        utl::hash_map<u64, FShaderReflectionUniform> uniforms{};
    };

    struct FShaderReflectionUnit
    {
        std::string name{};
        EShaderReflectionScopeType scope{};

        // In witch shaders it used (onty for local scope)
        utl::vector<u32> usages{};

        // TODO: add something like permutation or something

        EShaderReflectionBlockType blockType{};
        u32 shaderStages{};
        
        u32 memberRef{};
    };

    struct FShaderReflection
    {
        utl::vector<FShaderReflectionBlock> reflectedBlocks;

        utl::vector<FShaderReflectionBlock> pushBlock;
        utl::vector<FShaderReflectionBlock> uniformBlocks;
        utl::vector<FShaderReflectionBlock> storageBlocks;
    };

    struct FShaderCacheData
    {
        utl::hash_map<u32, FCompiledLibrary> libraries{};
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


    inline void to_json(nlohmann::json& json, const FShaderData& type)
    {
        utl::parsing::serialize_from("path", json, type.path, true);
    }

    inline void from_json(const nlohmann::json& json, FShaderData& type)
    {
        utl::parsing::parse_to("path", json, type.path);
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
    struct adl_serializer<utl::hash_map<u32, _Ty>>
    {
        static void to_json(json& j, const utl::hash_map<u32, _Ty>& map)
        {
            for (const auto& [key, value] : map)
                j[std::to_string(key)] = value;
        }

        static void from_json(const json& j, utl::hash_map<u32, _Ty>& map)
        {
            for (const auto& item : j.items())
            {
                u32 key = std::stoul(item.key());
                map[key] = item.value().get<_Ty>();
            }
        }
    };
}