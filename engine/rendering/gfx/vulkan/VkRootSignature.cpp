#include "VkRootSignature.h"
#include "VkDevice.h"
#include "VkDebug.h"
#include "VkSharedDescriptor.h"
#include "VkShader.h"

#include "VkUtil.h"

namespace engine
{
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
				VulkanResource{ nullptr, device.GetAllocationCallbacks() },
				device{ device }
			{
				vk::Result result = Handle(device).createPipelineLayout(&create_info.create_info, allocation_callbacks, &handle);
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


			VulkanRootSignature::VulkanRootSignature(Device& device, const RootSignatureCreateInfo& create_info) :
				device{ device }
			{
				const auto& reflection = create_info.shader->GetReflection();

				std::array<DescriptorSetLayoutCreateInfo, 8ull> descriptor_set_layout_create_infos{};

				// Create descriptor set layouts
				for (auto it = reflection.shader_resources.Begin(); it != reflection.shader_resources.End(); ++it)
				{
					auto& ci = descriptor_set_layout_create_infos[it->set];

					auto& descriptor_binding = ci.AddBinding();
					descriptor_binding.binding = it->binding;
					descriptor_binding.descriptorType = DescriptorTypeToVk(it->type);
					descriptor_binding.descriptorCount = 1u;
					descriptor_binding.stageFlags = ShaderStageFlagsToVk(it->stages);

					auto& flags = ci.AddBindingFlags();
				}

				PipelineLayoutCreateInfo layout_ci{};

				// Get constant ranges from reflection
				// TODO: Now i can have only one constant range, but in future i think i can have more then one.
				if (reflection.constants.range_begin != ~0u)
				{
					auto& range = layout_ci.AddConstantRange();
					for (int32_t i = reflection.constants.range_begin; i < reflection.constants.variable_count; ++i)
					{
						auto& variable = reflection.shader_variables.data[i];
						range.stageFlags = ShaderStageFlagsToVk(reflection.constants.stages);
						range.size += variable.size;
					}
				}

				// Allocate descriptr set layouts
				for (int32_t set = 0; set < 8ull; ++set)
				{
					// NOTE: it's very dirty solution i think, but ok for now
					if (create_info.shared_descriptor)
					{
						auto* vk_shared_descriptor = static_cast<VulkanSharedDescriptor*>(create_info.shared_descriptor);
						auto shared_set = vk_shared_descriptor->GetFirstSet();
						if (set == shared_set)
						{
							auto& shared_layout = vk_shared_descriptor->GetDescriptorSetLayout();
							layout_ci.AddDescriptorSetLayout(shared_layout.GetHandle());
						}
					}

					auto& ci = descriptor_set_layout_create_infos[set];
					if (ci.create_info.bindingCount > 0)
					{
						descriptor_set_layouts[set] = std::make_unique<DescriptorSetLayout>(device, ci);
						layout_ci.AddDescriptorSetLayout(descriptor_set_layouts[set]->GetHandle());
					}
				}

				// Add all immutable samplers
				// TODO: immutable samplers creation
				for (int32_t i = 0; i < create_info.immutable_sampler_count; ++i)
				{
					assert(false && "NOT IMPLEMENTED");
				}

				pipeline_layout = std::make_unique<PipelineLayout>(device, layout_ci);
			}

			VulkanRootSignature::~VulkanRootSignature()
			{

			}

			VulkanRootSignature::VulkanRootSignature(VulkanRootSignature&& other) :
				pipeline_layout{ std::exchange(other.pipeline_layout, {}) },
				device{ other.device }
			{
			}

			void VulkanRootSignature::SetResourceName(const char* name)
			{
				auto& debug_utils = device.GetDebugUtils();
				debug_utils.SetDebugName(pipeline_layout->GetHandle(), name);
			}

			PipelineLayout& VulkanRootSignature::GetPipelineLayout()
			{
				return *pipeline_layout;
			}

			const std::array<std::unique_ptr<DescriptorSetLayout>, k_max_descriptor_count>& VulkanRootSignature::GetDescriptorSetLayouts()
			{
				return descriptor_set_layouts;
			}
		}
	}
}