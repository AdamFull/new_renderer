#include <DescriptorUpdate.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			vk::WriteDescriptorSet& DescriptorUpdate::AddDescriptorWrite()
			{
				return write_descriptor_sets[writes_count++];
			}

			vk::DescriptorImageInfo& DescriptorUpdate::AddImageInfo()
			{
				return image_descriptors[image_count++];
			}

			vk::DescriptorBufferInfo& DescriptorUpdate::AddBufferInfo()
			{
				return buffer_descriptors[buffer_count++];
			}

			void DescriptorUpdate::Clear()
			{
				writes_count = 0u;
				image_count = 0u;
				buffer_count = 0u;
			}
		}
	}
}