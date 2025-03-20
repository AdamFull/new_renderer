#pragma once

#include "VkDescriptorSet.h"
#include "../gfx_base.h"

#include <deque>

namespace edge::gfx::vulkan
{
	class Device;

	class VulkanResourceDescriptor final : public IResourceDescriptor
	{
	public:
		VulkanResourceDescriptor(Device& device);
		~VulkanResourceDescriptor() = default;

		VulkanResourceDescriptor(const VulkanResourceDescriptor&) = delete;

		VulkanResourceDescriptor(VulkanResourceDescriptor&& other);

		VulkanResourceDescriptor& operator=(const VulkanResourceDescriptor&) = delete;
		VulkanResourceDescriptor& operator=(VulkanResourceDescriptor&&) = delete;

		void update_descriptor_set(uint32_t set, DescriptorSet& descriptor_set);

		auto get_first_set() const noexcept -> uint32_t {
			return first_set;
		}

		auto get_set_count() const noexcept -> uint32_t {
			return set_count;
		}

		void set_resource_name(std::string_view name) const noexcept override;

		void bind_texture(TextureDescriptor& texture_descriptor) override;
		void bind_buffer(BufferDescriptor& buffer_descriptor) override;

		void flush() override;

		[[nodiscard]] auto get_descriptor_sets() const noexcept -> std::array<vk::DescriptorSet, k_max_descriptor_count> const& {
			return descriptor_sets;
		}

	private:
		Device& device;

		uint32_t first_set{ ~0u };
		uint32_t set_count{ 0u };

		std::array<vk::DescriptorSet, k_max_descriptor_count> descriptor_sets{};

		std::vector<vk::WriteDescriptorSet> write_descriptor_sets{};
		std::deque<vk::DescriptorImageInfo> image_descriptors{};
		std::deque<vk::DescriptorBufferInfo> buffer_descriptors{};
	};
}