#include "VkImGuiIntegration.h"

#include <engine/render/render_api/vulkan/VkRenderDevice.h>
#include <engine/render/render_api/vulkan/VkRenderContext.h>

#include "imgui_impl_vulkan.h"

using namespace engine::render::vulkan;

void check_vk_result(vk::Result err)
{
    if (err == 0)
        return;

    log_error("[ImGui Integration] Error: vk::Result = %d\n", err);
}

void CVkImGuiIntegration::init()
{
    auto& device = m_pRenderContext->getDevice();

    FVkDescriptorPoolCreateInfo pool_ci{};
    pool_ci.poolSizes.emplace_back(vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, 1});

#ifndef BUILD_WITHOUT_DEBUGGIND
    pool_ci.name = "Imgui_integration";
#endif

    // TODO: add error logging
    auto err = device->createDescriptorPool(pool_ci, m_descriptorPool);

    auto& physical_device = device->getPhysicalDevice();
    auto& logical_device = device->getLogicalDevice();
    auto& swapchain = device->getSwapchain();

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = device->getInstance();
    init_info.PhysicalDevice = physical_device.handle;
    init_info.Device = logical_device.handle;
    init_info.QueueFamily = physical_device.queueFamilies.queueToFamily[EQueueTypeFlagBits::ePresent];
    init_info.Queue = logical_device.queues[EQueueTypeFlagBits::ePresent];
    init_info.PipelineCache = device->getPipelineCache();
    init_info.DescriptorPool = m_descriptorPool.handle;
    //init_info.RenderPass = wd->RenderPass;
    //init_info.Subpass = 0;
    init_info.UseDynamicRendering = true;
    init_info.MinImageCount = swapchain.framesInFlight;
    init_info.ImageCount = init_info.MinImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = device->getAllocationCallbacks();
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info);
}

void CVkImGuiIntegration::destroy()
{
    auto& device = m_pRenderContext->getDevice();

    ImGui_ImplVulkan_Shutdown();

    device->destroyDescriptorPool(m_descriptorPool);
}

void CVkImGuiIntegration::preRender()
{
    ImGui_ImplVulkan_NewFrame();
}

void CVkImGuiIntegration::postRender()
{
    auto& command_buffer = m_pRenderContext->getLastCommandBuffer(EQueueTypeFlagBits::eGraphics);

    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer.get());
}