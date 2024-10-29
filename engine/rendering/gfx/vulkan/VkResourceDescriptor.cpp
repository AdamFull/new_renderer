#include "VkResourceDescriptor.h"
#include "VkDevice.h"
#include "VkBuffer.h"
#include "VkTexture.h"
#include "VkSampler.h"

#include "VkUtil.h"

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			VulkanResourceDescriptor::VulkanResourceDescriptor(Device& device) :
				device{ device }
			{

			}

			VulkanResourceDescriptor::VulkanResourceDescriptor(VulkanResourceDescriptor&& other) :
				device{ device }, 
				first_set{ std::exchange(other.first_set, {}) },
				set_count{ std::exchange(other.set_count, {}) },
				descriptor_sets{ std::exchange(other.descriptor_sets, {}) },
				write_descriptor_sets{ std::exchange(other.write_descriptor_sets, {}) },
				writes_count{ std::exchange(other.writes_count, {}) },
				image_descriptors{ std::exchange(other.image_descriptors, {}) },
				image_count{ std::exchange(other.image_count, {}) },
				buffer_descriptors{ std::exchange(other.buffer_descriptors, {}) },
				buffer_count{ std::exchange(other.buffer_count, {}) }
			{
			}

			void VulkanResourceDescriptor::UpdateDescriptorSet(uint32_t set, DescriptorSet& descriptor_set)
			{
				first_set = std::min(first_set, set);
				descriptor_sets[set] = descriptor_set.GetHandle();
				set_count++;
			}

			uint32_t VulkanResourceDescriptor::GetFirstSet() const
			{
				return first_set;
			}

			uint32_t VulkanResourceDescriptor::GetSetCount() const
			{
				return set_count;
			}

			void VulkanResourceDescriptor::BindTexture(TextureDescriptor& texture_descriptor)
			{
				auto* vk_texture = static_cast<VulkanTexture*>(texture_descriptor.texture);
				auto* vk_sampelr = static_cast<VulkanSampler*>(texture_descriptor.sampler);

				auto& subresource = vk_texture->GetSubresource(texture_descriptor.subresource_index);

				auto& vk_image_descriptor = image_descriptors[image_count++];
				vk_image_descriptor.imageView = subresource.GetHandle();
				vk_image_descriptor.imageLayout = subresource.GetImageLayout();
				vk_image_descriptor.sampler = vk_sampelr->GetHandle();

				auto& vk_descriptor_write = write_descriptor_sets[writes_count++];
				vk_descriptor_write.descriptorType = DescriptorTypeToVk(texture_descriptor.type);
				vk_descriptor_write.descriptorCount = 1u;
				vk_descriptor_write.dstSet = descriptor_sets[texture_descriptor.set];
				vk_descriptor_write.dstBinding = texture_descriptor.binding;
				vk_descriptor_write.dstArrayElement = texture_descriptor.array_index;
				vk_descriptor_write.pImageInfo = &vk_image_descriptor;
			}

			void VulkanResourceDescriptor::BindBuffer(BufferDescriptor& buffer_descriptor)
			{
				auto* vk_buffer = static_cast<VulkanBuffer*>(buffer_descriptor.buffer);

				auto& vk_buffer_descriptor = buffer_descriptors[buffer_count++];
				vk_buffer_descriptor.buffer = vk_buffer->GetHandle();
				vk_buffer_descriptor.offset = buffer_descriptor.offset;
				vk_buffer_descriptor.range = buffer_descriptor.size ? buffer_descriptor.size : vk_buffer->GetSize();

				auto& vk_descriptor_write = write_descriptor_sets[writes_count++];
				vk_descriptor_write.descriptorType = DescriptorTypeToVk(buffer_descriptor.type);
				vk_descriptor_write.descriptorCount = 1u;
				vk_descriptor_write.dstSet = descriptor_sets[buffer_descriptor.set];
				vk_descriptor_write.dstBinding = buffer_descriptor.binding;
				vk_descriptor_write.pBufferInfo = &vk_buffer_descriptor;

				// TODO:
				vk_descriptor_write.dstArrayElement;
			}

			void VulkanResourceDescriptor::Flush()
			{
				Handle(device).updateDescriptorSets(writes_count, write_descriptor_sets.data(), 0, nullptr);

				writes_count = 0u;
				image_count = 0u;
				buffer_count = 0u;
				set_count = 0;
			}

			const std::array<vk::DescriptorSet, k_max_descriptor_count>& VulkanResourceDescriptor::GetDescriptorSets()
			{
				return descriptor_sets;
			}
		}
	}
}