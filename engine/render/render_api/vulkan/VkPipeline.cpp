#include <VkPipeline.h>
#include <VkDevice.h>

#include <VkShader.h>
#include <VkRootSignature.h>
#include <VkPipelineCache.h>

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			VulkanPipeline::VulkanPipeline(Device& device) :
				VulkanResource{ nullptr, device.GetAllocationCallbacks() },
				device{ device }
			{
			}

			VulkanPipeline::VulkanPipeline(VulkanPipeline&& other) :
				VulkanResource{ std::move(other) },
				device{ device }
			{
			}

			VulkanPipeline::~VulkanPipeline()
			{
				Handle(device).destroyPipeline(handle, allocation_callbacks);
			}



			VulkanComputePipeline::VulkanComputePipeline(Device& device, const ComputePipelineCreateInfo& create_info) :
				VulkanPipeline{ device }
			{
				auto* shader = static_cast<VulkanShader*>(create_info.shader);
				auto* root_signature = static_cast<VulkanRootSignature*>(create_info.root_signature);
				auto* pipeline_cache = static_cast<VulkanPipelineCache*>(create_info.pipeline_cache);

				vk::ComputePipelineCreateInfo pipeline_create_info{};

				auto& shader_modules = shader->GetModules();
				pipeline_create_info.stage = shader_modules.data[0]->GetStageCreateInfo();

				auto& pipeline_layout = root_signature->GetPipelineLayout();
				pipeline_create_info.layout = pipeline_layout.GetHandle();

				vk::PipelineCache cache{ pipeline_cache ? pipeline_cache->GetHandle() : nullptr };
				vk::Result result = Handle(device).createComputePipelines(cache, 1u, &pipeline_create_info, allocation_callbacks, &handle);
			}

			VulkanComputePipeline::VulkanComputePipeline(VulkanComputePipeline&& other) :
				VulkanPipeline{ std::move(other) }
			{

			}

			PipelineType VulkanComputePipeline::GetType() const
			{
				return PipelineType::eCompute;
			}




			VulkanGraphicsPipeline::VulkanGraphicsPipeline(Device& device, const GraphicsPipelineCreateInfo& create_info) :
				VulkanPipeline{ device }
			{
				auto* shader = static_cast<VulkanShader*>(create_info.shader);
				auto* root_signature = static_cast<VulkanRootSignature*>(create_info.root_signature);
				auto* pipeline_cache = static_cast<VulkanPipelineCache*>(create_info.pipeline_cache);

				std::array<vk::PipelineShaderStageCreateInfo, 5ull> shader_stages{};
				vk::GraphicsPipelineCreateInfo pipeline_create_info{};
				pipeline_create_info.pStages = shader_stages.data();

				auto& shader_modules = shader->GetModules();
				for (auto it = shader_modules.Begin(); it != shader_modules.End(); ++it)
				{
					shader_stages[pipeline_create_info.stageCount++] = (*it)->GetStageCreateInfo();
				}

				auto& pipeline_layout = root_signature->GetPipelineLayout();
				pipeline_create_info.layout = pipeline_layout.GetHandle();

				vk::PipelineCache cache{ pipeline_cache ? pipeline_cache->GetHandle() : nullptr };
				vk::Result result = Handle(device).createGraphicsPipelines(cache, 1u, &pipeline_create_info, allocation_callbacks, &handle);
			}

			VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanGraphicsPipeline&& other) :
				VulkanPipeline{ std::move(other) }
			{
			}

			PipelineType VulkanGraphicsPipeline::GetType() const
			{
				return PipelineType::eGraphics;
			}
		}
	}
}