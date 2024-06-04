#include "ImGuiIntegration.h"

#include "render/vulkan/VkImGuiIntegration.h"
#include "window/glfw/GLFWImGuiIntegration.h"

#include <imgui.h>

using namespace engine::integration;

void CImGuiIntegration::create()
{
    ImGuiIO& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    io.BackendRendererUserData = (void*)this;
    io.BackendRendererName = "vulkan";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can Create multi-viewports on the Renderer side (optional)

    // Create shader
    // Create font texture (just load) May be needs to implement texture interface
    // Create vertex handler with flag that will not generate meshlets (need to generate vertex attribute data also)

}

void CImGuiIntegration::destroy()
{

}

void CImGuiIntegration::render()
{

}