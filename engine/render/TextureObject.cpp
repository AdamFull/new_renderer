#include "TextureObject.h"

using namespace engine::render;

CTextureObject::CTextureObject(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

CTextureObject::~CTextureObject()
{

}

void CTextureObject::Create(const std::string& name, const FTextureCreateInfo& create_info)
{

}

void CTextureObject::Load(const std::string& path)
{

}