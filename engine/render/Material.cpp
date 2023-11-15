#include "Material.h"

#include "ShaderObject.h"

using namespace engine::render;

CMaterial::CMaterial()
{
	m_pShader = std::make_unique<CShaderObject>();
}

void CMaterial::addTexture(const char* name, size_t texture_id)
{
	m_pShader->bindTexture(name, texture_id);
}

void CMaterial::bind()
{

}