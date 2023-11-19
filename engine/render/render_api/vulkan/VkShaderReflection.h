#pragma once

#include <vulkan/vulkan.hpp>
#include <interface/render/RenderInterface.h>
#include <optional>

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
				std::vector<vk::SpecializationMapEntry> entries{};
				vk::SpecializationInfo info{};
			};

			struct FVkShaderReflectionData
			{
				std::vector<std::array<std::optional<uint32_t>, 3>> localSizes;
				uint32_t executionModeInvocations{ 0 };
				uint32_t executionModeOutputVertices{ 0 };

				std::unordered_map<size_t, FVkReflectionUniform> uniforms;
				std::unordered_map<size_t, FVkReflectionUniformBlock> uniformBlocks;
				std::unordered_map<size_t, FVkReflectionPushConstantBlock> pushBlocks;
				std::unordered_map<size_t, FVkReflectionSpecializationConstant> specializationConstants;

				std::vector<char> specializationDataBuffer{};

				std::unordered_map<uint32_t, std::vector<vk::DescriptorSetLayoutBinding>> descriptorSetLayoutBindings;
				std::vector<vk::DescriptorPoolSize> descriptorPoolSizes;
				std::unordered_map<vk::ShaderStageFlagBits, FVkSpecializationConstantData> specializationConstantStages{};
			};

			class CVkShaderReflectionBuilder
			{
			public:
				void addShaderStage(vk::ShaderStageFlagBits shaderStage, const std::vector<uint32_t>& shaderCode);
				FVkShaderReflectionData build();
			private:
				FVkShaderReflectionData reflectionData{};
				std::vector<vk::ShaderStageFlagBits> shaderStages{};
			};
		}
	}
}