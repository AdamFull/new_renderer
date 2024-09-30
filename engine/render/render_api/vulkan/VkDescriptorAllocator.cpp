//#include "VkDescriptorAllocator.h"
//#include "VkRenderDevice.h"
//
//using namespace engine::render::vulkan;
//
//CVkDescriptorAllocator::CVkDescriptorAllocator()
//{
//	initialize();
//}
//
//CVkDescriptorAllocator::CVkDescriptorAllocator(VkRenderDevice* device)
//{
//	render_device = device;
//	initialize();
//}
//
//bool CVkDescriptorAllocator::allocate(FVkDescriptorSet& descriptor_set, const vk::DescriptorSetLayout& layout)
//{
//	if (!m_current.handle)
//	{
//		m_current = grab();
//		m_used.emplace_back(m_current);
//	}
//
//	//FVkDescriptorSetCreateInfo set_ci{};
//	//set_ci.
//	return false;
//}
//
//void CVkDescriptorAllocator::reset()
//{
//	for (auto& pool : m_used)
//		render_device->ResetDescriptorPool(pool);
//
//	m_free = m_used;
//	m_used.clear();
//	m_current.handle = nullptr;
//}
//
//void CVkDescriptorAllocator::cleanup()
//{
//	for (auto& pool : m_free)
//		render_device->DestroyDescriptorPool(pool);
//
//	for (auto& pool : m_used)
//		render_device->DestroyDescriptorPool(pool);
//}
//
//void CVkDescriptorAllocator::initialize()
//{
//	m_sizes = {
//		{ vk::DescriptorType::eSampler, 0.5f },
//		{ vk::DescriptorType::eCombinedImageSampler, 4.f },
//		{ vk::DescriptorType::eSampledImage, 4.f },
//		{ vk::DescriptorType::eStorageImage, 1.f },
//		{ vk::DescriptorType::eUniformTexelBuffer, 1.f },
//		{ vk::DescriptorType::eStorageTexelBuffer, 1.f },
//		{ vk::DescriptorType::eUniformBuffer, 2.f },
//		{ vk::DescriptorType::eStorageBuffer, 2.f },
//		{ vk::DescriptorType::eUniformBufferDynamic, 1.f },
//		{ vk::DescriptorType::eStorageBufferDynamic, 1.f },
//		{ vk::DescriptorType::eInputAttachment, 0.5f }
//	};
//}
//
//FVkDescriptorPool CVkDescriptorAllocator::grab()
//{
//	if (!m_free.empty())
//	{
//		auto pool = m_free.back();
//		m_free.pop_back();
//		return pool;
//	}
//
//	FVkDescriptorPool new_pool{};
//	FVkDescriptorPoolCreateInfo pool_ci{};
//	// TODO: handle flags
//
//		//check_api_result(render_device->CreateDescriptorPool());
//	return new_pool;
//}