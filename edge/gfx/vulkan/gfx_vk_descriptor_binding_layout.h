#pragma once

#include "../gfx_base.h"
#include "gfx_vk_resource.h"

namespace edge::gfx::vulkan {
	class Device;

	class DescriptorBindingLayout final : 
		public IBindingLayout, public Resource<vk::DescriptorSet> {
	public:
		DescriptorBindingLayout(Device& device, IDescriptorLayout* descriptor_layout);
		DescriptorBindingLayout(const DescriptorBindingLayout&) = delete;
		DescriptorBindingLayout(DescriptorBindingLayout&& other);
		~DescriptorBindingLayout();

		DescriptorBindingLayout& operator=(const DescriptorBindingLayout&) = delete;
		DescriptorBindingLayout& operator=(DescriptorBindingLayout&&) = delete;

		void set_resource_name(std::string_view name) const noexcept override;

		auto valid() const noexcept -> bool override;

		auto bind(const SamplerDescriptor& sampler_desc) -> void override;
		auto bind(const TextureDescriptor& texture_desc) -> void override;
		auto bind(const BufferDescriptor& buffer_desc) -> void override;

		auto get_descriptor_layout() -> IDescriptorLayout* override {
			return descriptor_layout;
		}

		auto collect_updates(std::vector<vk::WriteDescriptorSet>& descriptor_writes) -> void;
		void mark_updated();

		auto set_descriptor_set(vk::DescriptorSet new_set, int32_t pool_idx) -> void;

		auto get_descriptor_set() const noexcept -> std::tuple<vk::DescriptorSet, int32_t> {
			return { handle_, descriptor_pool_idx };
		}
	private:
		Device& device;

		IDescriptorLayout* descriptor_layout{ nullptr };

		int32_t descriptor_pool_idx{ -1 };
		bool is_invalidated{ false };

		std::vector<vk::WriteDescriptorSet> write_descriptor_sets{};
		std::deque<vk::DescriptorImageInfo> image_descriptors{};
		std::deque<vk::DescriptorBufferInfo> buffer_descriptors{};
	};
}