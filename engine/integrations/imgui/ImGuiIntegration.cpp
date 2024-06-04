#include "ImGuiIntegration.h"

#include <engine/render/RenderInterface.h>
#include <engine/render/ShaderObject.h>
#include <engine/render/VertexHandler.h>

#include <imgui.h>

using namespace engine::integration;
using namespace engine::render;

void CImGuiIntegration::create()
{
    ImGuiIO& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    io.BackendRendererUserData = (void*)this;
    io.BackendRendererName = "vulkan";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
    //io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can Create multi-viewports on the Renderer side (optional)


    // Create vertex handler
    FVertexHandlerCreateInfo vertex_handler_ci{};
    vertex_handler_ci.indexSize = 2ull;
    vertex_handler_ci.descriptions.emplace_back(FVertexHandlerAttributeDescription{ 8ull, EVertexAttributeType::ePosition });
    vertex_handler_ci.descriptions.emplace_back(FVertexHandlerAttributeDescription{ 8ull, EVertexAttributeType::eTexCoord });
    vertex_handler_ci.descriptions.emplace_back(FVertexHandlerAttributeDescription{ 16ull, EVertexAttributeType::eColor });
    vertex_handler_ci.name = "imgui_vertices";
    m_vertexHandler = m_pRenderInterface->CreateVertexHandler("imgui_vertices", vertex_handler_ci);

    auto& vertex_handler = m_pRenderInterface->GetVertexHandler(m_vertexHandler);
    auto& render_context = m_pRenderInterface->GetRenderContext();

    // Create shader
    FGraphicsShaderCreateInfo shader_ci{};
    shader_ci.primitiveTopology = EVertexPrimitiveTopology::eTriangleList;
    shader_ci.cullMode = EShaderCullModeFlagBits::eNone;
    shader_ci.frontFace = EShaderFrontFace::eCCW;
    shader_ci.colorAttachments.emplace_back(FAttachment{ ~0ull, true });
    shader_ci.vertexDeclaration = vertex_handler->GetVertexDeclaration();
    m_shader = m_pRenderInterface->CreateShader("testing:imgui", shader_ci);

    // Create font texture (just load) May be needs to implement texture interface
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    u64 upload_size = width * height * 4 * sizeof(char);

    FTextureCreateInfo create_info{};
    create_info.format = EApiDataFormat::eRGBA8_UNORM;
    create_info.typeFlags = ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eTransferable | ETextureCreateFlagBits::e2D;
    create_info.SetFixedSize2D(width, height);

    FTextureUploadInfo upload_info{};
    upload_info.data.insert(upload_info.data.end(), pixels, pixels + upload_size);

    m_fontTexture = render_context->CreateTexture("imgui_font", create_info, std::move(upload_info));
}

void CImGuiIntegration::destroy()
{

}

void CImGuiIntegration::render()
{

}