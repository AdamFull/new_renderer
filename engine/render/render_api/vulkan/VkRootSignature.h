#pragma once

#include <GFXBase.h>
#include <VkResource.h>

namespace gfx
{
	namespace vulkan
	{
		class Device;

		class PipelineLayout final : public VulkanResource<vk::PipelineLayout>
		{
		public:
			PipelineLayout(Device& device);
			~PipelineLayout();

			PipelineLayout(PipelineLayout&) = delete;
			PipelineLayout(PipelineLayout&& other);

			PipelineLayout& operator=(const PipelineLayout&) = delete;
			PipelineLayout& operator=(PipelineLayout&&) = delete;
		private:
			Device& device;
		};


		class VulkanRootSignature final : public RootSignature
		{
		public:
			VulkanRootSignature(Device& device, const RootSignatureCreateInfo& create_info);
			~VulkanRootSignature();

			VulkanRootSignature(VulkanRootSignature&) = delete;
			VulkanRootSignature(VulkanRootSignature&& other);

			VulkanRootSignature& operator=(const VulkanRootSignature&) = delete;
			VulkanRootSignature& operator=(VulkanRootSignature&&) = delete;
		private:
			std::unique_ptr<PipelineLayout> pipeline_layout{};
		};
	}
}