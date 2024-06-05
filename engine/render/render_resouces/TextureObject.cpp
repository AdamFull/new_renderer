#include "TextureObject.h"

using namespace engine::render;

CTextureObject::CTextureObject(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

CTextureObject::~CTextureObject()
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->DestroyTexture(m_resourceId);
}

void CTextureObject::Create(const std::string& name, const FTextureCreateInfo& create_info)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	m_resourceId = render_context->CreateTexture(name, create_info);
}

void CTextureObject::Create(const std::string& name, const FTextureCreateInfo& create_info, FTextureUploadInfo&& upload_info)
{
	auto& render_context = m_pRenderInterface->GetRenderContext(); 
	m_resourceId = render_context->CreateTexture(name, create_info, std::move(upload_info));
}