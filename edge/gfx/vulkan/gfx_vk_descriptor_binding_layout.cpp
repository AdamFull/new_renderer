#include "gfx_vk_descriptor_binding_layout.h"
#include "gfx_vk_debug.h"
#include "gfx_vk_device.h"

#include "gfx_vk_buffer.h"
#include "gfx_vk_texture.h"
#include "gfx_vk_sampler.h"

#include "gfx_vk_descriptor_set_layout.h"

#include "gfx_vk_util.h"

#include "../../util/utl_profiler.h"

namespace edge::gfx::vulkan {
	DescriptorBindingLayout::DescriptorBindingLayout(Device& device, IDescriptorLayout* descriptor_layout) :
		Resource{ nullptr, device.get_allocation_callbacks() },
		device{ device },
		descriptor_layout{ descriptor_layout } {

		image_descriptors.reserve(512);
		buffer_descriptors.reserve(512);
	}

	DescriptorBindingLayout::DescriptorBindingLayout(DescriptorBindingLayout&& other) :
		Resource{ std::move(other) },
		device{ other.device },
		descriptor_layout{ std::exchange(other.descriptor_layout, nullptr) },
		descriptor_pool_idx{ std::exchange(other.descriptor_pool_idx, {}) },
		write_descriptor_sets{ std::exchange(other.write_descriptor_sets, {}) },
		image_descriptors{ std::exchange(other.image_descriptors, {}) },
		buffer_descriptors{ std::exchange(other.buffer_descriptors, {}) } {

	}

	DescriptorBindingLayout::~DescriptorBindingLayout() {

	}

	void DescriptorBindingLayout::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();
		debug_utils.set_debug_name(handle_, name);
	}

	auto DescriptorBindingLayout::valid() const noexcept -> bool {
		return !is_invalidated;
	}

	auto DescriptorBindingLayout::bind(const SamplerDescriptor& sampler_desc) -> void {
		assert(handle_ && "Descriptor set is not allocated.");
		auto& descriptor = image_descriptors.emplace_back();
		descriptor.sampler = sampler_desc.sampler->as<Sampler*>()->get_handle();

		auto& descriptor_write = write_descriptor_sets.emplace_back();
		descriptor_write.dstSet = handle_;
		descriptor_write.dstBinding = sampler_desc.binding;
		descriptor_write.dstArrayElement = sampler_desc.array_index;
		descriptor_write.descriptorType = vk::DescriptorType::eSampler;
		descriptor_write.descriptorCount = 1u;
		descriptor_write.pImageInfo = &descriptor;
	}

	auto DescriptorBindingLayout::bind(const TextureDescriptor& texture_desc) -> void {
		assert(handle_ && "Descriptor set is not allocated.");

		auto image_desc_offset = image_descriptors.size();
		uint32_t descritpro_count = 0u;

		auto* vk_texture = texture_desc.texture->as<Texture*>();
		if (texture_desc.type == DescriptorType::eStorageImage) {
			if (texture_desc.subresource_index != ~0u) {
				auto& subresource = vk_texture->get_uav_view(texture_desc.subresource_index);

				auto& descriptor = image_descriptors.emplace_back();
				descriptor.imageView = subresource.get_handle();
				descriptor.imageLayout = resource_state_to_vk_image_layout(texture_desc.resource_state);
				descritpro_count = 1;
			}
			else {
				auto view_count = vk_texture->get_mip_level_count();
				for (int32_t i = 0; i < view_count; ++i) {
					auto& subresource = vk_texture->get_uav_view(i);

					auto& descriptor = image_descriptors.emplace_back();
					descriptor.imageView = subresource.get_handle();
					descriptor.imageLayout = resource_state_to_vk_image_layout(texture_desc.resource_state);
					descritpro_count++;
				}
			}
		}
		else {
			auto& subresource = vk_texture->get_srv_view();

			auto& descriptor = image_descriptors.emplace_back();
			descriptor.imageView = subresource.get_handle();
			descriptor.imageLayout = resource_state_to_vk_image_layout(texture_desc.resource_state);
			descritpro_count = 1;
		}

		auto& vk_descriptor_write = write_descriptor_sets.emplace_back();
		vk_descriptor_write.descriptorType = descriptor_type_to_vk(texture_desc.type);
		vk_descriptor_write.descriptorCount = descritpro_count;
		vk_descriptor_write.dstSet = handle_;
		vk_descriptor_write.dstBinding = texture_desc.binding;
		vk_descriptor_write.dstArrayElement = texture_desc.array_index;
		vk_descriptor_write.pImageInfo = image_descriptors.data() + image_desc_offset;
	}

	auto DescriptorBindingLayout::bind(const BufferDescriptor& buffer_desc) -> void {
		assert(handle_ && "Descriptor set is not allocated.");

		auto* vk_buffer = buffer_desc.buffer->as<Buffer*>();

		auto& vk_buffer_descriptor = buffer_descriptors.emplace_back();
		vk_buffer_descriptor.buffer = vk_buffer->get_handle();
		vk_buffer_descriptor.offset = buffer_desc.offset;
		vk_buffer_descriptor.range = buffer_desc.size ? buffer_desc.size : vk_buffer->get_size();

		auto& vk_descriptor_write = write_descriptor_sets.emplace_back();
		vk_descriptor_write.descriptorType = descriptor_type_to_vk(buffer_desc.type);
		vk_descriptor_write.descriptorCount = 1u;
		vk_descriptor_write.dstSet = handle_;
		vk_descriptor_write.dstBinding = buffer_desc.binding;
		vk_descriptor_write.pBufferInfo = &vk_buffer_descriptor;
	}

	// TODO: merge updates if it's possible
	auto DescriptorBindingLayout::collect_updates(std::vector<vk::WriteDescriptorSet>& descriptor_writes) -> void {
		descriptor_writes.insert(descriptor_writes.end(), write_descriptor_sets.begin(), write_descriptor_sets.end());
		write_descriptor_sets.clear();
	}

	void DescriptorBindingLayout::mark_updated() {
		if (buffer_descriptors.empty() && image_descriptors.empty()) {
			return;
		}

		buffer_descriptors.clear();
		image_descriptors.clear();

		auto* vk_descriptor_layout = descriptor_layout->as<DescriptorSetLayout*>();
		if (!vk_descriptor_layout->is_bindless()) {
			is_invalidated = true;
		}
	}

	auto DescriptorBindingLayout::set_descriptor_set(vk::DescriptorSet new_set, int32_t pool_idx) -> void {
		is_invalidated = !new_set;
		handle_ = new_set;
		descriptor_pool_idx = pool_idx;
	}
}