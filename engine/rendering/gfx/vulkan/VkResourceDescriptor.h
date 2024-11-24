#pragma once

#include "VkDescriptorSet.h"
#include <Base.h>

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			class Device;

			class VulkanResourceDescriptor final : public ResourceDescriptor
			{
			public:
				VulkanResourceDescriptor(Device& device);
				~VulkanResourceDescriptor() = default;

				VulkanResourceDescriptor(const VulkanResourceDescriptor&) = delete;

				VulkanResourceDescriptor(VulkanResourceDescriptor&& other);

				VulkanResourceDescriptor& operator=(const VulkanResourceDescriptor&) = delete;
				VulkanResourceDescriptor& operator=(VulkanResourceDescriptor&&) = delete;

				void UpdateDescriptorSet(uint32_t set, DescriptorSet& descriptor_set);

				uint32_t GetFirstSet() const;
				uint32_t GetSetCount() const;

				void BindTexture(TextureDescriptor& texture_descriptor) override;
				void BindBuffer(BufferDescriptor& buffer_descriptor) override;

				void Flush() override;

				const std::array<vk::DescriptorSet, k_max_descriptor_count>& GetDescriptorSets();

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