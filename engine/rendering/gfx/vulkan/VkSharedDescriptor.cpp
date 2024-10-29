#include "VkSharedDescriptor.h"
#include "VkDevice.h"
#include "VkDescriptorPool.h"
#include "VkResourceDescriptor.h"

#include "VkUtil.h"

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			VulkanSharedDescriptor::VulkanSharedDescriptor(Device& device, const SharedDescriptorCreateInfo& create_info) :
				device{ device },
				first_set{ create_info .first_set }
			{
				DescriptorSetLayoutCreateInfo layout_ci{};
				layout_ci.create_info.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPoolEXT;

				for (auto& desc : create_info.descriptions)
				{
					auto& binding = layout_ci.AddBinding();
					binding.descriptorType = DescriptorTypeToVk(desc.type);
					binding.binding = desc.binding;
					binding.descriptorCount = desc.count;
					binding.stageFlags = vk::ShaderStageFlagBits::eAllGraphics | vk::ShaderStageFlagBits::eCompute;

					auto& flags = layout_ci.AddBindingFlags();
					flags = { vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind };
				}

				descriptor_set_layout = std::make_unique<DescriptorSetLayout>(device, layout_ci);

				descriptor_pool = std::make_unique<DescriptorPool>(device, *descriptor_set_layout);
			}

			VulkanSharedDescriptor::~VulkanSharedDescriptor()
			{
				for (auto& resource_descriptor : allocated_resource_descriptors)
				{
					delete resource_descriptor;
					resource_descriptor = nullptr;
				}
			}

			VulkanSharedDescriptor::VulkanSharedDescriptor(VulkanSharedDescriptor&& other) :
				device{ other.device },
				first_set{ std::exchange(other.first_set, {}) },
				descriptor_set_layout{ std::exchange(other.descriptor_set_layout , {}) },
				descriptor_pool{ std::exchange(other.descriptor_pool , {}) }
			{

			}

			DescriptorSetLayout& VulkanSharedDescriptor::GetDescriptorSetLayout()
			{
				return *descriptor_set_layout;
			}

			//DescriptorSet& VulkanSharedDescriptor::GetDescriptorSet()
			//{
			//
			//}

			uint32_t VulkanSharedDescriptor::GetFirstSet() const
			{
				return first_set;
			}

			void VulkanSharedDescriptor::Reset()
			{
				descriptor_pool->Reset();

				for (auto& resource_descriptor : allocated_resource_descriptors)
				{
					delete resource_descriptor;
					resource_descriptor = nullptr;
				}
				allocated_resource_descriptors.clear();
			}

			ResourceDescriptor* VulkanSharedDescriptor::GetLatestDescriptors()
			{
				return allocated_resource_descriptors.back();
			}

			ResourceDescriptor* VulkanSharedDescriptor::RequestDescriptors()
			{
				auto& new_resource_descriptor = allocated_resource_descriptors.emplace_back(new VulkanResourceDescriptor(device));

				auto& descriptor_set = descriptor_pool->Allocate(*descriptor_set_layout);
				new_resource_descriptor->UpdateDescriptorSet(first_set, descriptor_set);

				return new_resource_descriptor;
			}
		}
	}
}