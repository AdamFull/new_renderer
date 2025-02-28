#pragma once

#include "VkDescriptorSet.h"
#include "../gfx_base.h"

#include <deque>

namespace edge
{
	namespace gfx
	{
		namespace vulkan
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

				void UpdateDescriptorSet(uint32_t set, DescriptorSet& descriptor_set);

				auto GetFirstSet() const noexcept -> uint32_t
				{
					return first_set;
				}

				auto GetSetCount() const noexcept -> uint32_t
				{
					return set_count;
				}

				void SetResourceName(std::string_view name) const noexcept override;

				void BindTexture(TextureDescriptor& texture_descriptor) override;
				void BindBuffer(BufferDescriptor& buffer_descriptor) override;

				void Flush() override;

				[[nodiscard]] auto GetDescriptorSets() const noexcept -> std::array<vk::DescriptorSet, k_max_descriptor_count> const&
				{
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
	}
}