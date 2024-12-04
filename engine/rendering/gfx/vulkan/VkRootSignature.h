#pragma once

#include "../gfx_base.h"
#include "VkDescriptorSetLayout.h"

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			class Device;

			struct PipelineLayoutCreateInfo
			{
			public:
				vk::PipelineLayoutCreateInfo create_info{};

				std::array<vk::DescriptorSetLayout, 8ull> descriptor_set_layouts{};
				std::array<vk::PushConstantRange, 16ull> push_constant_ranges{};

				PipelineLayoutCreateInfo();

				void AddDescriptorSetLayout(vk::DescriptorSetLayout layout);
				vk::PushConstantRange& AddConstantRange();
			};

			class PipelineLayout final : public VulkanResource<vk::PipelineLayout>
			{
			public:
				PipelineLayout(Device& device, const PipelineLayoutCreateInfo& create_info);
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
				friend class VulkanDescriptorAllocator;
				friend class VulkanCommandList;

				VulkanRootSignature(Device& device, const RootSignatureCreateInfo& create_info);
				~VulkanRootSignature();

				VulkanRootSignature(VulkanRootSignature&) = delete;
				VulkanRootSignature(VulkanRootSignature&& other);

				VulkanRootSignature& operator=(const VulkanRootSignature&) = delete;
				VulkanRootSignature& operator=(VulkanRootSignature&&) = delete;

				void SetResourceName(const char* name) override;

				PipelineLayout& GetPipelineLayout();
				const std::array<std::unique_ptr<DescriptorSetLayout>, k_max_descriptor_count>& GetDescriptorSetLayouts();
			private:
				Device& device;

				std::unique_ptr<PipelineLayout> pipeline_layout{};
				std::array<std::unique_ptr<DescriptorSetLayout>, k_max_descriptor_count> descriptor_set_layouts{};
			};
		}
	}
}