#pragma once

// Based on https://github.com/vblanco20-1/vulkan-guide/blob/engine/extra-engine/vk_descriptors.h

//#include "VkResourceStructures.h"
//
//namespace engine
//{
//	namespace render
//	{
//		namespace vulkan
//		{
//			// fwd
//			class VkRenderDevice;
//
//			class CVkDescriptorAllocator
//			{
//			public:
//				CVkDescriptorAllocator();
//				CVkDescriptorAllocator(VkRenderDevice* device);
//
//				bool allocate(FVkDescriptorSet& descriptor_set, const vk::DescriptorSetLayout& layout);
//				void reset();
//				void cleanup();
//
//			protected:
//				void initialize();
//			private:
//				FVkDescriptorPool grab();
//
//				FVkDescriptorPool m_current{};
//				utl::vector<FVkDescriptorPool> m_used{};
//				utl::vector<FVkDescriptorPool> m_free{};
//				utl::hash_map<vk::DescriptorType, f32> m_sizes{};
//
//				VkRenderDevice* render_device{ nullptr };
//			};
//
//			class CVkDescriptorLayoutCache
//			{
//
//			};
//
//			class CVkDescriptorBuilder
//			{
//
//			};
//		}
//	}
//}