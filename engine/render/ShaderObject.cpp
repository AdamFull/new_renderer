#include "ShaderObject.h"

using namespace engine::render;

CShaderObject::CShaderObject(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

CShaderObject::~CShaderObject()
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->DestroyShader(m_shaderId);
}

void CShaderObject::Create(const std::string& name, const FGraphicsShaderCreateInfo& create_info)
{
	m_shaderId = m_pRenderInterface->CreateShader(name, create_info);
	BuildReflection();
}

void CShaderObject::Create(const std::string& name, const FComputeShaderCreateInfo& create_info)
{
	m_shaderId = m_pRenderInterface->CreateShader(name, create_info);
	BuildReflection();
}

const std::unique_ptr<CUniformHandle>& CShaderObject::GetUniformBlock(u64 name)
{
	static std::unique_ptr<CUniformHandle> null{ nullptr };

	auto found = m_uniformBlocks.find(name);
	if (found != m_uniformBlocks.end())
		return found->second;

	return null;
}

const std::unique_ptr<CUniformHandle>& CShaderObject::GetConstantBlock(u64 name)
{
	static std::unique_ptr<CUniformHandle> null{ nullptr };

	auto found = m_constantBlocks.find(name);
	if (found != m_constantBlocks.end())
		return found->second;

	return null;
}

void CShaderObject::BindBuffer(u64 name, FAPIHandle buffer_resource)
{
	m_bindBuffers[name] = buffer_resource;
}

void CShaderObject::BindTexture(u64 name, FAPIHandle texture_resource, u32 mip)
{
	m_bindTextures[name] = FTextureBindInfo{ texture_resource, mip };
}

void CShaderObject::FlushDescriptors()
{
	// Universal block for binding external resources
	auto& external_resources = GetUniformBlock(shader_bindings::external_bind_resources);
	if (external_resources)
	{
		auto& render_graph = m_pRenderInterface->GetRenderGraph();
		// TODO: Required multiple changes for bindless system inside api implementation
		// start_element_index + (layer_index * layer.mips.size()) + mip_index;
	}

	FShaderResourceCommit update_data{};
	update_data.shaderId = m_shaderId;
	update_data.textures = m_bindTextures;
	update_data.buffers = m_bindBuffers;

	// Collect uniform buffers and storage buffers
	for (auto& [id, block] : m_uniformBlocks)
	{
		auto& data_view = update_data.uniforms[id];
		auto& block_data = block->Data();
		data_view.data = block_data.data();
		data_view.size = block_data.size();
	}

	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->UpdateShaderResources(update_data);
}

void CShaderObject::FlushConstants(EQueueTypeFlagBits queue)
{
	utl::hash_map<u64, FUniformDataView> constants{};

	for (auto& [id, block] : m_constantBlocks)
	{
		auto& data_view = constants[id];
		auto& block_data = block->Data();
		data_view.data = block_data.data();
		data_view.size = block_data.size();
	}

	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->FlushConstantRanges(m_shaderId, constants, queue);
}

void CShaderObject::Bind(EQueueTypeFlagBits queue)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->BindShader(m_shaderId, queue);
}

void CShaderObject::BindResources(EQueueTypeFlagBits queue)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->BindShaderResources(m_shaderId, queue);
}

FAPIHandle CShaderObject::Handle() const
{
	return m_shaderId;
}

const FShaderReflectionData& CShaderObject::GetReflection() const
{
	return m_reflection;
}

void CShaderObject::BuildReflection()
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	m_reflection = render_context->GetShaderReflection(m_shaderId);

	for (auto& [id, uniform_block] : m_reflection.uniformBlocks)
	{
		auto uniform_handle = std::make_unique<CUniformHandle>();
		uniform_handle->Create(uniform_block);
		m_uniformBlocks[id] = std::move(uniform_handle);
	}

	for (auto& [id, push_block] : m_reflection.pushBlocks)
	{
		auto uniform_handle = std::make_unique<CUniformHandle>();
		uniform_handle->Create(push_block);
		m_constantBlocks[id] = std::move(uniform_handle);
	}
}