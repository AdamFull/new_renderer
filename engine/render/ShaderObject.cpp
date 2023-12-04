#include "ShaderObject.h"

using namespace engine::render;

CShaderObject::CShaderObject(IRenderInterface* render_interface, IRenderContext* render_context)
{
	m_pRenderInterface = render_interface;
	m_pRenderContext = render_context;
}

CShaderObject::~CShaderObject()
{
	m_pRenderContext->destroyShader(m_shaderId);
}

void CShaderObject::create(const std::string& name, const FGraphicsShaderCreateInfo& create_info)
{
	m_shaderId = m_pRenderInterface->createShader(name, create_info);
	buildReflection();
}

void CShaderObject::create(const std::string& name, const FComputeShaderCreateInfo& create_info)
{
	m_shaderId = m_pRenderInterface->createShader(name, create_info);
	buildReflection();
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

void CShaderObject::bindBuffer(const char* name, size_t buffer_resource)
{
	m_bindBuffers[utl::fnv1a_64_hash(name)] = buffer_resource;
}

void CShaderObject::bindTexture(const char* name, size_t texture_resource)
{
	m_bindTextures[utl::fnv1a_64_hash(name)] = texture_resource;
}

void CShaderObject::flushDescriptors()
{
	FShaderResourceCommit update_data{};
	update_data.shaderId = m_shaderId;
	update_data.textures = m_bindTextures;
	update_data.buffers = m_bindBuffers;

	// Collect uniform buffers and storage buffers
	for (auto& [id, block] : m_uniformBlocks)
	{
		auto& data_view = update_data.uniforms[id];
		auto& block_data = block->data();
		data_view.data = block_data.data();
		data_view.size = block_data.size();
	}

	m_pRenderContext->updateShaderResources(update_data);
}

void CShaderObject::flushConstants(uint32_t queue)
{
	std::unordered_map<size_t, FUniformDataView> constants{};

	for (auto& [id, block] : m_constantBlocks)
	{
		auto& data_view = constants[id];
		auto& block_data = block->data();
		data_view.data = block_data.data();
		data_view.size = block_data.size();
	}

	m_pRenderContext->flushConstantRanges(m_shaderId, constants, queue);
}

void CShaderObject::bind(uint32_t queue)
{
	m_pRenderContext->bindShader(m_shaderId, queue);
}

const FShaderReflectionData& CShaderObject::getReflection() const
{
	return m_reflection;
}

void CShaderObject::buildReflection()
{
	m_reflection = m_pRenderContext->getShaderReflection(m_shaderId);

	for (auto& [id, uniform_block] : m_reflection.uniformBlocks)
	{
		auto uniform_handle = std::make_unique<CUniformHandle>();
		uniform_handle->create(uniform_block);
		m_uniformBlocks[id] = std::move(uniform_handle);
	}

	for (auto& [id, push_block] : m_reflection.pushBlocks)
	{
		auto uniform_handle = std::make_unique<CUniformHandle>();
		uniform_handle->create(push_block);
		m_constantBlocks[id] = std::move(uniform_handle);
	}
}