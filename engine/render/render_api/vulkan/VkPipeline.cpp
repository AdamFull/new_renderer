#include <VkPipeline.h>
#include <VkDevice.h>

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