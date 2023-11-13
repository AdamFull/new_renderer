#include "ShaderObject.h"

using namespace engine::render;

void CShaderObject::create(size_t resource, const FShaderReflectionData& reflection)
{
	m_shaderId = resource;

	for (auto& [id, uniform_block] : reflection.uniformBlocks)
	{
		auto uniform_handle = std::make_unique<CUniformHandle>();
		uniform_handle->create(uniform_block);
		m_uniformBlocks[id] = std::move(uniform_handle);
	}

	for (auto& [id, push_block] : reflection.pushBlocks)
	{
		auto uniform_handle = std::make_unique<CUniformHandle>();
		uniform_handle->create(push_block);
		m_constantBlocks[id] = std::move(uniform_handle);
	}
}

const std::unique_ptr<CUniformHandle>& CShaderObject::getUniformBlock(const char* name)
{
	static std::unique_ptr<CUniformHandle> null{ nullptr };

	auto found = m_uniformBlocks.find(utl::fnv1a_64_hash(name));
	if (found != m_uniformBlocks.end())
		return found->second;

	return null;
}

const std::unique_ptr<CUniformHandle>& CShaderObject::getConstantBlock(const char* name)
{
	static std::unique_ptr<CUniformHandle> null{ nullptr };

	auto found = m_constantBlocks.find(utl::fnv1a_64_hash(name));
	if (found != m_constantBlocks.end())
		return found->second;

	return null;
}

void CShaderObject::bindTexture(const char* name, size_t texture_resource)
{
	m_bindTextures[utl::fnv1a_64_hash(name)] = texture_resource;
}

FShaderResourceCommit CShaderObject::getResourceCommit()
{
	FShaderResourceCommit update_data{};
	update_data.shaderId = m_shaderId;
	update_data.textures = m_bindTextures;

	// Collect uniform buffers and storage buffers
	for (auto& [id, block] : m_uniformBlocks)
	{
		auto& data_view = update_data.uniforms[id];
		auto& block_data = block->data();
		data_view.data = block_data.data();
		data_view.size = block_data.size();
	}

	// Collect push blocks
	for (auto& [id, block] : m_constantBlocks)
	{
		auto& data_view = update_data.constants[id];
		auto& block_data = block->data();
		data_view.data = block_data.data();
		data_view.size = block_data.size();
	}

	return update_data;
}

size_t CShaderObject::getId()
{
	return m_shaderId;
}