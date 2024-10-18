#pragma once

#include <vulkan/vulkan.hpp>
#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			struct FVkReflectionUniform : FReflectionUniform
			{
				vk::ShaderStageFlags stageFlags{ vk::ShaderStageFlagBits::eAll };
				vk::DescriptorType descriptorType{};
			};

			struct FVkReflectionUniformBlock : FReflectionUniformBlock<FVkReflectionUniform>
			{
				vk::ShaderStageFlags stageFlags{ vk::ShaderStageFlagBits::eAll };
				vk::DescriptorType descriptorType;
			};

			struct FVkReflectionPushConstantBlock : FReflectionPushConstantBlock<FVkReflectionUniform>
			{
				vk::ShaderStageFlags stageFlags{ vk::ShaderStageFlagBits::eAll };
			};

			struct FVkReflectionSpecializationConstant
			{
				uint32_t constant_id{ 0u };
				size_t size{ 0ull };
				uint32_t offset{ 0u };
				vk::ShaderStageFlags stageFlags{ vk::ShaderStageFlagBits::eAll };
			};

			struct FVkSpecializationConstantData
			{
				utl::vector<vk::SpecializationMapEntry> entries{};
				vk::SpecializationInfo info{};
			};

			struct FVkShaderReflectionData
			{
				utl::vector<std::array<uint32_t, 3ull>> localSizes;
				uint32_t executionModeInvocations{ 0 };
				uint32_t executionModeOutputVertices{ 0 };

				utl::hash_map<size_t, FVkReflectionUniform> uniforms;
				utl::hash_map<size_t, FVkReflectionUniformBlock> uniformBlocks;
				utl::hash_map<size_t, FVkReflectionPushConstantBlock> pushBlocks;
				utl::hash_map<size_t, FVkReflectionSpecializationConstant> specializationConstants;

				utl::vector<char> specializationDataBuffer{};

				utl::hash_map<uint32_t, utl::vector<vk::DescriptorSetLayoutBinding>> descriptorSetLayoutBindings;

				static constexpr size_t k_max_pool_sizes{ 20ull };
				std::array<vk::DescriptorPoolSize, k_max_pool_sizes> descriptor_pool_sizes;
				size_t pool_sizes_count{ 0ull };

				utl::hash_map<vk::ShaderStageFlagBits, FVkSpecializationConstantData> specializationConstantStages{};
			};

			class CVkShaderReflectionBuilder
			{
			public:
				void AddShaderStage(vk::ShaderStageFlagBits shaderStage, std::span<uint32_t> shader_code);
				FVkShaderReflectionData Build();
			private:
				FVkShaderReflectionData reflectionData{};
				utl::vector<vk::ShaderStageFlagBits> shaderStages{};
			};
		}
	}
}