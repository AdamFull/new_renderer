#include "gfx_vk_resource_descriptor.h"
#include "gfx_vk_device.h"
#include "gfx_vk_buffer.h"
#include "gfx_vk_texture.h"
#include "gfx_vk_sampler.h"

#include "gfx_vk_util.h"

#include "../../util/Profiler.h"

namespace edge::gfx::vulkan
{
	VulkanResourceDescriptor::VulkanResourceDescriptor(Device& device) :
		device{ device } {
	}

	VulkanResourceDescriptor::VulkanResourceDescriptor(VulkanResourceDescriptor&& other) :
		device{ other.device },
		first_set{ std::exchange(other.first_set, {}) },
		set_count{ std::exchange(other.set_count, {}) },
		descriptor_sets{ std::exchange(other.descriptor_sets, {}) },
		write_descriptor_sets{ std::exchange(other.write_descriptor_sets, {}) },
		image_descriptors{ std::exchange(other.image_descriptors, {}) },
		buffer_descriptors{ std::exchange(other.buffer_descriptors, {}) } {
	}

	void VulkanResourceDescriptor::update_descriptor_set(uint32_t set, DescriptorSet& descriptor_set) {
		first_set = std::min(first_set, set);
		descriptor_sets[set] = descriptor_set.get_handle();
		set_count++;
	}

	void VulkanResourceDescriptor::set_resource_name(std::string_view name) const noexcept {
		assert(false && "NOT IMPLEMENTED");
	}

	void VulkanResourceDescriptor::bind_texture(TextureDescriptor& texture_descriptor) {
		auto& vk_image_descriptor = image_descriptors.emplace_back();

		auto descriptor_type = descriptor_type_to_vk(texture_descriptor.type);

		if (descriptor_type != vk::DescriptorType::eSampler) {
			auto* vk_texture = static_cast<VulkanTexture*>(texture_descriptor.texture);
			auto& subresource = vk_texture->get_subresource(texture_descriptor.subresource_index);
			vk_image_descriptor.imageView = subresource.get_handle();
			vk_image_descriptor.imageLayout = resource_state_to_vk_image_layout(texture_descriptor.resource_state);
		}

		if (descriptor_type != vk::DescriptorType::eStorageImage &&
			descriptor_type != vk::DescriptorType::eSampledImage) {
			auto* vk_sampelr = static_cast<VulkanSampler*>(texture_descriptor.sampler);
			vk_image_descriptor.sampler = vk_sampelr->get_handle();
		}

		auto& vk_descriptor_write = write_descriptor_sets.emplace_back();
		vk_descriptor_write.descriptorType = descriptor_type;
		vk_descriptor_write.descriptorCount = 1u;
		vk_descriptor_write.dstSet = descriptor_sets[texture_descriptor.set];
		vk_descriptor_write.dstBinding = texture_descriptor.binding;
		vk_descriptor_write.dstArrayElement = texture_descriptor.array_index;
		vk_descriptor_write.pImageInfo = &vk_image_descriptor;
	}

	void VulkanResourceDescriptor::bind_buffer(BufferDescriptor& buffer_descriptor) {
		auto* vk_buffer = static_cast<VulkanBuffer*>(buffer_descriptor.buffer);

		auto& vk_buffer_descriptor = buffer_descriptors.emplace_back();
		vk_buffer_descriptor.buffer = vk_buffer->get_handle();
		vk_buffer_descriptor.offset = buffer_descriptor.offset;
		vk_buffer_descriptor.range = buffer_descriptor.size ? buffer_descriptor.size : vk_buffer->get_size();

		auto& vk_descriptor_write = write_descriptor_sets.emplace_back();
		vk_descriptor_write.descriptorType = descriptor_type_to_vk(buffer_descriptor.type);
		vk_descriptor_write.descriptorCount = 1u;
		vk_descriptor_write.dstSet = descriptor_sets[buffer_descriptor.set];
		vk_descriptor_write.dstBinding = buffer_descriptor.binding;
		vk_descriptor_write.pBufferInfo = &vk_buffer_descriptor;

		// TODO:
		vk_descriptor_write.dstArrayElement;
	}

	void VulkanResourceDescriptor::flush() {
		EDGE_PROFILE_ZONE_SCOPED;

		device.get_handle().updateDescriptorSets(write_descriptor_sets.size(), write_descriptor_sets.data(), 0, nullptr);
	}
}