#include "VkShaderReflection.h"

#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>

using namespace engine::render::vulkan;

bool has_substring(const std::string& str, const std::string& target)
{
    return std::search(str.begin(), str.end(), target.begin(), target.end(), [](char a, char b) { return std::tolower(a) == std::tolower(b); }) != str.end();
}

u64 get_basic_type_size(spirv_cross::CompilerGLSL* compiler, const spirv_cross::SPIRType& type)
{
    if (type.basetype == spirv_cross::SPIRType::Struct)
        return compiler->get_declared_struct_size(type);

    return type.width / 8;
}

void parse_execution_mode(FVkShaderReflectionData& reflection_data, spirv_cross::CompilerGLSL* compiler, u32 index)
{
    auto mode = utl::cast<spv::ExecutionMode>(index);
    switch (mode)
    {
    case spv::ExecutionMode::ExecutionModeInvocations: reflection_data.executionModeInvocations = compiler->get_execution_mode_argument(mode, 0); break;
    case spv::ExecutionMode::ExecutionModeLocalSize:
    {
        auto& sizes = reflection_data.localSizes.emplace_back();
        for (u32 dim = 0u; dim < 3u; ++dim)
            sizes[dim] = compiler->get_execution_mode_argument(mode, dim);
    } break;
    case spv::ExecutionMode::ExecutionModeOutputVertices: reflection_data.executionModeOutputVertices = compiler->get_execution_mode_argument(mode, 0); break;
    }
}

void build_specialization_constant_reflection(FVkShaderReflectionData& reflection_data, spirv_cross::CompilerGLSL* compiler, const spirv_cross::SpecializationConstant& spec_const, vk::ShaderStageFlagBits shaderStage)
{
    auto name = compiler->get_name(spec_const.id);

    u64 name_hash = utl::murmur_hash_64(name);
    auto it = reflection_data.specializationConstants.find(name_hash);
    if (it != reflection_data.specializationConstants.end())
    {
        it->second.stageFlags |= shaderStage;
        return;
    }

    auto& constant = compiler->get_constant(spec_const.id);
    auto& type = compiler->get_type(constant.constant_type);

    u64 offset = reflection_data.specializationDataBuffer.size();

    FVkReflectionSpecializationConstant specialization_constant{};
    specialization_constant.constant_id = spec_const.constant_id;
    specialization_constant.stageFlags = shaderStage;
    specialization_constant.size = get_basic_type_size(compiler, type);
    specialization_constant.offset = utl::cast<u32>(offset);

    reflection_data.specializationDataBuffer.resize(offset + specialization_constant.size);
    engine::util::Copy(reflection_data.specializationDataBuffer.data(), offset, &constant.m.c[0].r[0], specialization_constant.size);

    reflection_data.specializationConstants.emplace(name_hash, specialization_constant);
}

void build_uniform_block_reflection(FVkShaderReflectionData& reflection_data, spirv_cross::CompilerGLSL* compiler, const spirv_cross::Resource& res, vk::ShaderStageFlagBits shaderStage, vk::DescriptorType descriptorType)
{
    u64 name_hash = utl::murmur_hash_64(res.name);
    auto it = reflection_data.uniformBlocks.find(name_hash);
    if (it != reflection_data.uniformBlocks.end())
    {
        it->second.stageFlags |= shaderStage;
        return;
    }

    if (has_substring(res.name, "global"))
        return;

    const auto& type = compiler->get_type(res.type_id);
    auto& decoration = compiler->get_decoration_bitset(type.self);
    unsigned member_count = type.member_types.size();

    FVkReflectionUniformBlock uniform_block{};
    uniform_block.set = compiler->get_decoration(res.id, spv::DecorationDescriptorSet);
    uniform_block.binding = compiler->get_decoration(res.id, spv::DecorationBinding);

    for (u32 i = 0u; i < member_count; ++i)
    {
        u64 array_stride{ 0ull }, matrix_stride{ 0ull };
        auto& member_type = compiler->get_type(type.member_types[i]);

        FVkReflectionUniform uniform{};
        uniform.size = compiler->get_declared_struct_member_size(type, i);
        uniform.offset = compiler->type_struct_member_offset(type, i);
        if (!member_type.array.empty() && uniform.size == 0)
            array_stride = compiler->type_struct_member_array_stride(type, i);

        if (member_type.columns > 1)
            matrix_stride = compiler->type_struct_member_matrix_stride(type, i);

        uniform.stageFlags = shaderStage;

        u64 member_name_hash = utl::murmur_hash_64(compiler->get_member_name(type.self, i));
        uniform_block.uniforms.emplace(member_name_hash, uniform);
    }

    uniform_block.descriptorType = descriptorType;
    uniform_block.size = compiler->get_declared_struct_size(type);
    uniform_block.stageFlags = shaderStage;

    reflection_data.uniformBlocks.emplace(name_hash, uniform_block);
}

void build_push_constants_block_reflection(FVkShaderReflectionData& reflection_data, spirv_cross::CompilerGLSL* compiler, const spirv_cross::Resource& res, vk::ShaderStageFlagBits shaderStage)
{
    u64 name_hash = utl::murmur_hash_64(res.name);
    auto it = reflection_data.pushBlocks.find(name_hash);
    if (it != reflection_data.pushBlocks.end())
    {
        it->second.stageFlags |= shaderStage;
        return;
    }

    const auto& type = compiler->get_type(res.type_id);
    unsigned member_count = type.member_types.size();
    FVkReflectionPushConstantBlock push_block{};

    for (u32 i = 0; i < member_count; i++)
    {
        auto& member_type = compiler->get_type(type.member_types[i]);
        FVkReflectionUniform uniform{};
        uniform.size = compiler->get_declared_struct_member_size(type, i);
        uniform.offset = compiler->type_struct_member_offset(type, i);
        if (!member_type.array.empty()) 
            u64 array_stride = compiler->type_struct_member_array_stride(type, i);
        if (member_type.columns > 1) 
            u64 matrix_stride = compiler->type_struct_member_matrix_stride(type, i);
        uniform.stageFlags = shaderStage;

        u64 member_name_hash = utl::murmur_hash_64(compiler->get_member_name(type.self, i));
        push_block.uniforms.emplace(member_name_hash, uniform);
    }

    push_block.size = compiler->get_declared_struct_size(type);
    push_block.stageFlags = shaderStage;
    reflection_data.pushBlocks.emplace(name_hash, push_block);
}

void build_uniform_reflection(FVkShaderReflectionData& reflection_data, spirv_cross::CompilerGLSL* compiler, const spirv_cross::Resource& res, vk::ShaderStageFlagBits shaderStage, vk::DescriptorType descriptorType)
{
    u64 name_hash = utl::murmur_hash_64(res.name);
    auto it = reflection_data.uniforms.find(name_hash);
    if (it != reflection_data.uniforms.end())
    {
        it->second.stageFlags |= shaderStage;
        return;
    }

    if (has_substring(res.name, "global"))
        return;
       
    FVkReflectionUniform uniform{};
    uniform.set = compiler->get_decoration(res.id, spv::DecorationDescriptorSet);
    uniform.binding = compiler->get_decoration(res.id, spv::DecorationBinding);
    uniform.descriptorType = descriptorType;
    uniform.stageFlags = shaderStage;

    reflection_data.uniforms.emplace(name_hash, uniform);
}


void CVkShaderReflectionBuilder::AddShaderStage(vk::ShaderStageFlagBits shaderStage, std::span<uint32_t> shader_code)
{
	spirv_cross::CompilerGLSL compiler(shader_code.data(), shader_code.size());
    auto active = compiler.get_active_interface_variables();
    auto resources = compiler.get_shader_resources(active);
    compiler.set_enabled_interface_variables(std::move(active));

    // Parse execution modes
    auto& execution_modes = compiler.get_execution_mode_bitset();
    execution_modes.for_each_bit([&](u32 i) { parse_execution_mode(reflectionData, &compiler, i); });

    // Parse specialization constants
    for (const auto& spec_const : compiler.get_specialization_constants())
        build_specialization_constant_reflection(reflectionData, &compiler, spec_const, shaderStage);

    // Parse uniform buffers
    for (const auto& res : resources.uniform_buffers)
        build_uniform_block_reflection(reflectionData, &compiler, res, shaderStage, vk::DescriptorType::eUniformBuffer);

    // Parse storage buffers
    for (const auto& res : resources.storage_buffers)
        build_uniform_block_reflection(reflectionData, &compiler, res, shaderStage, vk::DescriptorType::eStorageBuffer);

    // Parse push constants blocks
    for (const auto& res : resources.push_constant_buffers)
        build_push_constants_block_reflection(reflectionData, &compiler, res, shaderStage);

    for (const auto& res : resources.sampled_images)
        build_uniform_reflection(reflectionData, &compiler, res, shaderStage, vk::DescriptorType::eCombinedImageSampler);

    for (const auto& res : resources.separate_images)
        build_uniform_reflection(reflectionData, &compiler, res, shaderStage, vk::DescriptorType::eSampledImage);

    for (const auto& res : resources.separate_samplers)
        build_uniform_reflection(reflectionData, &compiler, res, shaderStage, vk::DescriptorType::eSampler);

    for (const auto& res : resources.storage_images)
        build_uniform_reflection(reflectionData, &compiler, res, shaderStage, vk::DescriptorType::eStorageImage);

    for (const auto& res : resources.subpass_inputs)
        build_uniform_reflection(reflectionData, &compiler, res, shaderStage, vk::DescriptorType::eInputAttachment);

    shaderStages.emplace_back(shaderStage);
}

FVkShaderReflectionData CVkShaderReflectionBuilder::Build()
{
    utl::hash_map<vk::DescriptorType, u32> descriptorUsageCounter{};

    for (const auto& [name, block] : reflectionData.uniformBlocks)
    {
        vk::DescriptorSetLayoutBinding descriptor_binding{};
        descriptor_binding.binding = block.binding;
        descriptor_binding.descriptorType = block.descriptorType;
        descriptor_binding.descriptorCount = 1u;
        descriptor_binding.stageFlags = block.stageFlags;

        reflectionData.descriptorSetLayoutBindings[block.set].emplace_back(descriptor_binding);
        descriptorUsageCounter[block.descriptorType]++;
    }

    for (const auto& [name, uniform] : reflectionData.uniforms)
    {
        vk::DescriptorSetLayoutBinding descriptor_binding = {};
        descriptor_binding.binding = uniform.binding;
        descriptor_binding.descriptorType = uniform.descriptorType;
        descriptor_binding.descriptorCount = 1u;
        descriptor_binding.stageFlags = uniform.stageFlags;

        reflectionData.descriptorSetLayoutBindings[uniform.set].emplace_back(descriptor_binding);
        descriptorUsageCounter[uniform.descriptorType]++;
    }

    for (const auto& [descriptorType, count] : descriptorUsageCounter)
        reflectionData.descriptorPoolSizes.emplace_back(vk::DescriptorPoolSize{ descriptorType, count });

    // Sort descriptors by binding.
    for (auto& [set, descriptors] : reflectionData.descriptorSetLayoutBindings)
        std::sort(descriptors.begin(), descriptors.end(), [](const vk::DescriptorSetLayoutBinding& l, const vk::DescriptorSetLayoutBinding& r) { return l.binding < r.binding; });

    // Remove not unique elements
    for (auto& [set, descriptors] : reflectionData.descriptorSetLayoutBindings)
    {
        auto first = std::unique(descriptors.begin(), descriptors.end(), [](const vk::DescriptorSetLayoutBinding& l, const vk::DescriptorSetLayoutBinding& r) { return l.binding == r.binding; });

        if(first != descriptors.end())
            descriptors.erase(first, descriptors.end());
    }
    
    for (const auto& stage : shaderStages)
    {
        auto& constants_data = reflectionData.specializationConstantStages[stage];
        for (const auto& [name, constant] : reflectionData.specializationConstants)
        {
            auto constant_stage_flags = constant.stageFlags;
            if (constant_stage_flags & stage)
            {
                auto& entry = constants_data.entries.emplace_back();
                entry.constantID = constant.constant_id;
                entry.size = constant.size;
                entry.offset = constant.offset;
            }
        }

        constants_data.info.mapEntryCount = utl::cast<u32>(constants_data.entries.size());
        constants_data.info.pMapEntries = constants_data.entries.data();
        constants_data.info.dataSize = utl::cast<u32>(reflectionData.specializationDataBuffer.size());
        constants_data.info.pData = reflectionData.specializationDataBuffer.data();
    }

    return reflectionData;
}