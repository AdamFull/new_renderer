#include "gfx_vk_descriptor_set_layout.h"
#include "gfx_vk_debug.h"
#include "gfx_vk_device.h"

#include "gfx_vk_buffer.h"
#include "gfx_vk_texture.h"
#include "gfx_vk_sampler.h"

#include "gfx_vk_util.h"

#include "../../util/utl_profiler.h"

namespace edge::gfx::vulkan
{
	DescriptorSetLayout::DescriptorSetLayout(Device& device, const DescriptorLayoutCreateInfo& layout_ci) :
		Resource{ nullptr, device.get_allocation_callbacks() },
		device{ device } {

		vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT ext_create_info{};
		vk::DescriptorSetLayoutCreateInfo create_info{};
		create_info.pNext = &ext_create_info;

		std::vector<vk::DescriptorBindingFlagsEXT> binding_flags{};
		std::vector<vk::DescriptorSetLayoutBinding> bindings{};

		for (const auto& layout_binding : layout_ci.bindings) {
			auto& new_binding = bindings.emplace_back();
			new_binding.descriptorType = descriptor_type_to_vk(layout_binding.type);
			new_binding.binding = layout_binding.binding;
			new_binding.descriptorCount = layout_binding.count;
			new_binding.stageFlags = shader_stage_flags_to_vk(layout_binding.stages);

			auto& new_flags = binding_flags.emplace_back();
			if (layout_ci.support_bindless) {
				new_flags |= vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind;
			}

			pool_sizes.sizes[utl::cast(new_binding.descriptorType)] += new_binding.descriptorCount;

			layout_hash = gfx::hash(layout_binding, layout_hash);
		}

		binding_count = layout_ci.bindings.size();

		ext_create_info.pBindingFlags = binding_flags.data();
		ext_create_info.bindingCount = static_cast<uint32_t>(binding_flags.size());

		if (layout_ci.support_bindless) {
			have_bindless_descriptor = true;
			create_info.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
		}

		create_info.pBindings = bindings.data();
		
		create_info.bindingCount = static_cast<uint32_t>(bindings.size());

		// TODO: Add error check
		vk::Result result = device.get_handle().createDescriptorSetLayout(&create_info, allocation_callbacks, &handle_);
	}

	DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) :
		Resource{ std::move(other) },
		device{ other.device },
		binding_count{ std::exchange(other.binding_count, {}) },
		pool_sizes{ std::exchange(other.pool_sizes, {}) } {
	}

	DescriptorSetLayout::~DescriptorSetLayout() {
		device.get_handle()
			.destroyDescriptorSetLayout(handle_, allocation_callbacks);
	}

	void DescriptorSetLayout::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();
		debug_utils.set_debug_name(handle_, name);
		descriptor_name = name;
	}

	auto DescriptorSetLayout::is_bindless() const noexcept -> bool {
		return have_bindless_descriptor;
	}

	auto DescriptorSetLayout::valid() const noexcept -> bool {
		return !is_invalidated;
	}

	auto DescriptorSetLayout::bind(const SamplerDescriptor& sampler_desc) -> void {
		assert(descriptor_set && "Descriptor set is not allocated.");
		auto& descriptor = image_descriptors.emplace_back();
		descriptor.sampler = sampler_desc.sampler->as<Sampler*>()->get_handle();
		
		auto& descriptor_write = write_descriptor_sets.emplace_back();
		descriptor_write.dstSet = descriptor_set;
		descriptor_write.dstBinding = sampler_desc.binding;
		descriptor_write.dstArrayElement = sampler_desc.array_index;
		descriptor_write.descriptorType = vk::DescriptorType::eSampler;
		descriptor_write.descriptorCount = 1u;
		descriptor_write.pImageInfo = &descriptor;
	}

	auto DescriptorSetLayout::bind(const TextureDescriptor& texture_desc) -> void {
		assert(descriptor_set && "Descriptor set is not allocated.");

		auto& descriptor = image_descriptors.emplace_back();
		
		auto* vk_texture = texture_desc.texture->as<Texture*>();
		auto& subresource = vk_texture->get_subresource(texture_desc.subresource_index);
		descriptor.imageView = subresource.get_handle();
		descriptor.imageLayout = resource_state_to_vk_image_layout(texture_desc.resource_state);
		
		if (texture_desc.type != DescriptorType::eStorageImage &&
			texture_desc.type != DescriptorType::eSampledImage) {
			auto* vk_sampelr = texture_desc.sampler->as<Sampler*>();
			descriptor.sampler = vk_sampelr->get_handle();
		}
		
		auto& vk_descriptor_write = write_descriptor_sets.emplace_back();
		vk_descriptor_write.descriptorType = descriptor_type_to_vk(texture_desc.type);
		vk_descriptor_write.descriptorCount = 1u;
		vk_descriptor_write.dstSet = descriptor_set;
		vk_descriptor_write.dstBinding = texture_desc.binding;
		vk_descriptor_write.dstArrayElement = texture_desc.array_index;
		vk_descriptor_write.pImageInfo = &descriptor;
	}

	auto DescriptorSetLayout::bind(const BufferDescriptor& buffer_desc) -> void {
		assert(descriptor_set && "Descriptor set is not allocated.");

		volatile int iiii = 0;
		if (!descriptor_name.empty()) {
			++iiii;
		}

		auto* vk_buffer = buffer_desc.buffer->as<Buffer*>();
		
		auto& vk_buffer_descriptor = buffer_descriptors.emplace_back();
		vk_buffer_descriptor.buffer = vk_buffer->get_handle();
		vk_buffer_descriptor.offset = buffer_desc.offset;
		vk_buffer_descriptor.range = buffer_desc.size ? buffer_desc.size : vk_buffer->get_size();
		
		auto& vk_descriptor_write = write_descriptor_sets.emplace_back();
		vk_descriptor_write.descriptorType = descriptor_type_to_vk(buffer_desc.type);
		vk_descriptor_write.descriptorCount = 1u;
		vk_descriptor_write.dstSet = descriptor_set;
		vk_descriptor_write.dstBinding = buffer_desc.binding;
		vk_descriptor_write.pBufferInfo = &vk_buffer_descriptor;
	}

	auto DescriptorSetLayout::update() -> void {
		EDGE_PROFILE_ZONE_SCOPED;

		if (write_descriptor_sets.empty()) {
			return;
		}
		
		device.get_handle()
			.updateDescriptorSets(static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);

		// Clear any writes
		write_descriptor_sets.clear();
		buffer_descriptors.clear();
		image_descriptors.clear();

		// Update after bind should not invalidate descriptor set
		if (!have_bindless_descriptor) {
			is_invalidated = true;
		}
	}

	auto DescriptorSetLayout::set_descriptor_set(vk::DescriptorSet new_set, int32_t pool_idx) -> void {
		if (new_set && descriptor_set) {
			invalidated_sets.push_back(std::make_pair(descriptor_set, descriptor_pool_idx));
		}
		else {
			invalidated_sets.clear();
		}

		is_invalidated = !new_set;
		descriptor_set = new_set;
		descriptor_pool_idx = pool_idx;

		if (descriptor_set && !descriptor_name.empty()) {
			auto& debug_utils = device.get_debug_utils();
			debug_utils.set_debug_name(descriptor_set, descriptor_name);
		}
	}
}