#include "ShaderObject.h"

using namespace engine::render;

const std::unique_ptr<CUniformHandle>& CShaderObject::getUniformBlock(const char* name)
{
	static std::unique_ptr<CUniformHandle> null{ nullptr };

	auto found = uniformBlocks.find(utl::fnv1a_64_hash(name));
	if (found != uniformBlocks.end())
		return found->second;

	return null;
}

void CShaderObject::bindTexture(const char* name, FResourceView texture_resource)
{
	bindTextures[utl::fnv1a_64_hash(name)] = texture_resource;
}