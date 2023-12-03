#pragma once

// Based on https://github.com/vblanco20-1/vulkan-guide/blob/engine/extra-engine/vk_descriptors.h

#include "VkResourceStructures.h"

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			// fwd
			class CVkRenderDevice;

			class CVkDescriptorAllocator
			{
			public:
				CVkDescriptorAllocator();
				CVkDescriptorAllocator(CVkRenderDevice* device);

				bool allocate(FVkDescriptorSet& descriptor_set, const vk::DescriptorSetLayout& layout);
				void reset();
				void cleanup();

			protected:
				void initialize();
			private:
				FVkDescriptorPool grab();

				FVkDescriptorPool m_current{};
				std::vector<FVkDescriptorPool> m_used{};
				std::vector<FVkDescriptorPool> m_free{};
				std::unordered_map<vk::DescriptorType, float> m_sizes{};

				CVkRenderDevice* m_pDevice{ nullptr };
			};

			class CVkDescriptorLayoutCache
			{

			};

			class CVkDescriptorBuilder
			{

			};
		}
	}
}