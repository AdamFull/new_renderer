#include "TextureObject.h"

using namespace engine::render;

CTextureObject::CTextureObject(IRenderInterface* render_interface)
{
	m_pRenderContext = render_interface->GetRenderBackend().get();
}

CTextureObject::~CTextureObject()
{
	m_pRenderContext->DestroyTexture(m_textureId);
}

void CTextureObject::Create(const std::string& name, const FTextureCreateInfo& create_info)
{
	m_textureId = m_pRenderContext->CreateTexture(name, create_info);
}

void CTextureObject::Create(const std::string& name, const FTextureCreateInfo& create_info, FTextureUploadInfo&& upload_info)
{
	m_textureId = m_pRenderContext->CreateTexture(name, create_info, std::move(upload_info));
}

void CTextureObject::GetPixel(u32 x, u32 y, u8* data)
{
	m_pRenderContext->ReadPixel(m_textureId, x, y, data);
}

void CTextureObject::GetDimensions(u32* width, u32* height, u32* depth)
{
	auto texture_data = m_pRenderContext->GetTextureView(m_textureId);

	*width = texture_data.width;

	if (height)
		*height = texture_data.height;

	if (depth)
		*depth = texture_data.depth;
}

void CTextureObject::GetMipLevelCount(u32* mip_count)
{
	auto texture_data = m_pRenderContext->GetTextureView(m_textureId);
	*mip_count = texture_data.mipLevels;
}

void CTextureObject::GetLayerCount(u32* layer_count)
{
	auto texture_data = m_pRenderContext->GetTextureView(m_textureId);
	*layer_count = texture_data.arrayLayers;
}

FAPIHandle CTextureObject::Handle()
{
	return m_textureId;
}