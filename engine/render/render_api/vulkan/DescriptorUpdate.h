#pragma once

#include <vulkan/vulkan.hpp>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class DescriptorUpdate
			{
			public:
				std::array<vk::WriteDescriptorSet, 128ull> write_descriptor_sets{};
				uint32_t writes_count{ 0u };

				std::array<vk::DescriptorImageInfo, 64ull> image_descriptors{};
				uint32_t image_count{ 0u };

				std::array<vk::DescriptorBufferInfo, 64ull> buffer_descriptors{};
				uint32_t buffer_count{ 0u };

				vk::WriteDescriptorSet& AddDescriptorWrite();
				vk::DescriptorImageInfo& AddImageInfo();
				vk::DescriptorBufferInfo& AddBufferInfo();

				void Clear();
			};
		}
	}
}