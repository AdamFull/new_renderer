#include <VkRootSignature.h>
#include <VkDevice.h>

namespace gfx
{
	namespace vulkan
	{
		PipelineLayout::PipelineLayout(Device& device) :
			VulkanResource{nullptr, device.GetAllocationCallbacks() },
			device{ device }
		{

		}

		PipelineLayout::~PipelineLayout()
		{
			Handle(device).destroyPipelineLayout(handle, allocation_callbacks);
		}

		PipelineLayout::PipelineLayout(PipelineLayout&& other) :
			VulkanResource{ std::move(other) },
			device{ device }
		{
		}


		VulkanRootSignature::VulkanRootSignature(Device& device, const RootSignatureCreateInfo& create_info)
		{

		}

		VulkanRootSignature::~VulkanRootSignature()
		{

		}

		VulkanRootSignature::VulkanRootSignature(VulkanRootSignature&& other) :
			pipeline_layout{ std::exchange(other.pipeline_layout, {}) }
		{
		}
	}
}