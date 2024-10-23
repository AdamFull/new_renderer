#include <VkRootSignature.h>
#include <VkDevice.h>

#include <VkShader.h>

#include <VkUtil.h>

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
			const auto& reflection = create_info.shader->GetReflection();

			std::array<DescriptorSetLayoutCreateInfo, 8ull> descriptor_set_layout_create_infos{};

			// Create descriptor set layouts
			for (auto it = reflection.shader_resources.Begin(); it != reflection.shader_resources.End(); ++it)
			{
				auto& ci = descriptor_set_layout_create_infos[it->set];

				auto& descriptor_binding = ci.bindings[ci.create_info.bindingCount++];
				descriptor_binding.binding = it->binding;
				descriptor_binding.descriptorType = DescriptorPoolToVk(it->type);
				descriptor_binding.descriptorCount = 1u;
				descriptor_binding.stageFlags = ShaderStageFlagsToVk(it->stages);

				auto& flags = ci.binding_flags_ext[ci.ext_create_info.bindingCount++];
			}

			for (int32_t set = 0; set < 8ull; ++set)
			{
				auto& ci = descriptor_set_layout_create_infos[set];
				if (ci.create_info.bindingCount > 0)
				{
					descriptor_set_layouts[set] = std::make_unique<DescriptorSetLayout>(device, ci);
				}
			}
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