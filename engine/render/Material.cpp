#include "Material.h"

#include "ShaderObject.h"

using namespace engine::render;

CMaterial::CMaterial(IRenderInterface* render_interface, IRenderContext* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;

	m_pShader = std::make_unique<CShaderObject>(render_interface, render_context);
}

void CMaterial::addTexture(const char* name, size_t texture_id)
{
	m_pShader->bindTexture(name, texture_id);
}

void CMaterial::bind()
{

}