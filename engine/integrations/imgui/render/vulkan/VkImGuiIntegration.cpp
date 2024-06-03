#include "VkImGuiIntegration.h"

#include <logger/logger.h>
#include <engine/render/render_api/vulkan/VkResourceStructures.h>

#include "imgui_impl_vulkan.h"

using namespace engine::integration;
using namespace engine::render::vulkan;

void check_vk_result(vk::Result err)
{
    if (err == vk::Result::eSuccess)
        return;

    log_error("[ImGui Integration] Error: vk::Result = %d\n", (u32)err);
}

void CVkImGuiIntegration::Init()
{
    // TODO: Init m_pRenderInterface

    auto& render_context = m_pRenderInterface->GetRenderContext();

    FVkNativeContext native_context{};
    render_context->GetNativeContext(&native_context);

    m_device = native_context.device;
    m_allocationCallbacks = native_context.allocationCallbacks;

    // Create descriptor pool
    {
        utl::vector<vk::DescriptorPoolSize> pool_sizes{ vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, 1} };

        // Calculate total descriptor set count in descriptor pool and update descriptor sizes
        u32 maxSets{ 1u };
        for (auto& pool_size : pool_sizes)
        {
            pool_size.descriptorCount *= native_context.backbufferFrames;
            maxSets += pool_size.descriptorCount;
        }

        vk::DescriptorPoolCreateInfo descriptor_pool_ci{};
        descriptor_pool_ci.poolSizeCount = utl::cast<u32>(pool_sizes.size());
        descriptor_pool_ci.pPoolSizes = pool_sizes.data();
        descriptor_pool_ci.maxSets = maxSets;
        descriptor_pool_ci.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

#ifndef BUILD_WITHOUT_DEBUGGIND
        auto name = "ImGui descriptor pool";
        using object_type = decltype(m_descriptorPool);
        vk::DebugUtilsObjectNameInfoEXT object_name_info{};
        object_name_info.objectType = m_descriptorPool.objectType;
        object_name_info.objectHandle = reinterpret_cast<u64>(static_cast<object_type::CType>(m_descriptorPool));

        auto formatted_debug_object_name = std::format("{}:{} | {:x}", vk::to_string(m_descriptorPool.objectType), name, object_name_info.objectHandle);
        object_name_info.pObjectName = formatted_debug_object_name.c_str();

        native_context.device.setDebugUtilsObjectNameEXT(object_name_info);
#endif

        vk::Result res = native_context.device.createDescriptorPool(&descriptor_pool_ci, native_context.allocationCallbacks, &m_descriptorPool);
    }

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = native_context.instance;
    init_info.PhysicalDevice = native_context.physicalDevice;
    init_info.Device = native_context.device;
    init_info.QueueFamily = native_context.presentQueueFamily;
    init_info.Queue = native_context.presentQueue;
    init_info.PipelineCache = native_context.pipelineCache;
    init_info.DescriptorPool = m_descriptorPool;
    init_info.UseDynamicRendering = true;
    init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &native_context.backbufferFormat;
    init_info.MinImageCount = native_context.backbufferFrames;
    init_info.ImageCount = init_info.MinImageCount;
    init_info.MSAASamples = native_context.samples;
    init_info.Allocator = native_context.allocationCallbacks;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info);
}

void CVkImGuiIntegration::destroy()
{
    ImGui_ImplVulkan_Shutdown();
    m_device.destroyDescriptorPool(m_descriptorPool, m_allocationCallbacks);
}

void CVkImGuiIntegration::preRender()
{
    ImGui_ImplVulkan_NewFrame();
}

void CVkImGuiIntegration::postRender()
{
    ImDrawData* draw_data = ImGui::GetDrawData();
    //ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer.get());
}