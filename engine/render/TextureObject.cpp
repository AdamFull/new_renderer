#include "TextureObject.h"

using namespace engine::render;

CTextureObject::CTextureObject(IRenderInterface* render_interface)
{
	m_pRenderContext = render_interface->GetRenderContext().get();
}

CTextureObject::~CTextureObject()
{
	m_pRenderContext->DestroyTexture(m_textureId);
}

void CTextureObject::Create(const std::string& name, const FTextureCreateInfo& create_info)
{
	//m_textureId = m_pRenderContext->CreateTexture(name, )
}

void CTextureObject::Create(const std::string& name, const FTextureCreateInfo& create_info, FTextureUploadInfo&& upload_info)
{

}

FAPIHandle CTextureObject::Handle()
{
	return m_textureId;
}