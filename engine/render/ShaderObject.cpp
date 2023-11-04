#include "ShaderObject.h"

using namespace engine::render;

void CShaderObject::create(FResourceView resource, const FShaderReflectionData& reflection)
{
	for (auto& [id, uniform_block] : reflection.uniformBlocks)
	{
		auto uniform_handle = std::make_unique<CUniformHandle>();
		uniform_handle->create(uniform_block);
		uniformBlocks[id] = std::move(uniform_handle);
	}

	for (auto& [id, push_block] : reflection.pushBlocks)
	{
		auto uniform_handle = std::make_unique<CUniformHandle>();
		uniform_handle->create(push_block);
		uniformBlocks[id] = std::move(uniform_handle);
	}
}

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