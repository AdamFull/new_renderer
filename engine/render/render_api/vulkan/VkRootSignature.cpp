#include <VkRootSignature.h>
#include <VkDevice.h>

#include <VkShader.h>

#include <VkUtil.h>

namespace gfx
{
	namespace vulkan
	{
		PipelineLayoutCreateInfo::PipelineLayoutCreateInfo()
		{
			create_info.pSetLayouts = descriptor_set_layouts.data();
			create_info.pPushConstantRanges = push_constant_ranges.data();
		}

		void PipelineLayoutCreateInfo::AddDescriptorSetLayout(vk::DescriptorSetLayout layout)
		{
			descriptor_set_layouts[create_info.setLayoutCount++] = layout;
		}

		vk::PushConstantRange& PipelineLayoutCreateInfo::AddConstantRange()
		{
			return push_constant_ranges[create_info.pushConstantRangeCount++];
		}


		PipelineLayout::PipelineLayout(Device& device, const PipelineLayoutCreateInfo& create_info) :
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
			const auto& reflection = create_info.shader->GetReflection();

			std::array<DescriptorSetLayoutCreateInfo, 8ull> descriptor_set_layout_create_infos{};

			// Create descriptor set layouts
			for (auto it = reflection.shader_resources.Begin(); it != reflection.shader_resources.End(); ++it)
			{
				auto& ci = descriptor_set_layout_create_infos[it->set];

				auto& descriptor_binding = ci.AddBinding();
				descriptor_binding.binding = it->binding;
				descriptor_binding.descriptorType = DescriptorPoolToVk(it->type);
				descriptor_binding.descriptorCount = 1u;
				descriptor_binding.stageFlags = ShaderStageFlagsToVk(it->stages);

				auto& flags = ci.AddBindingFlags();
			}

			PipelineLayoutCreateInfo layout_ci{};

			// Get constant ranges from reflection
			if (reflection.constants.range_begin != ~0u)
			{
				for (int32_t i = reflection.constants.range_begin; i < reflection.constants.variable_count; ++i)
				{
					auto& variable = reflection.shader_variables.data[i];
					auto& range = layout_ci.AddConstantRange();
					range.stageFlags = ShaderStageFlagsToVk(reflection.constants.stages);
					range.size = variable.size;
					range.offset = variable.offset;
				}
			}

			// Allocate descriptr set layouts
			for (int32_t set = 0; set < 8ull; ++set)
			{
				auto& ci = descriptor_set_layout_create_infos[set];
				if (ci.create_info.bindingCount > 0)
				{
					descriptor_set_layouts[set] = std::make_unique<DescriptorSetLayout>(device, ci);
					layout_ci.AddDescriptorSetLayout(descriptor_set_layouts[set]->GetHandle());
				}
			}

			vk::PipelineLayoutCreateInfo pl_create_info{};
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