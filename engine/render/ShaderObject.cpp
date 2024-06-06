#include "ShaderObject.h"

using namespace engine::render;

CShaderObject::CShaderObject(IRenderInterface* render_interface)
{
	auto& render_context = render_interface->GetRenderContext();
	m_pRenderContext = render_context.get();
}

CShaderObject::~CShaderObject()
{
	m_pRenderContext->DestroyShader(m_shaderId);
}

void CShaderObject::Create(const std::string& name, const FGraphicsShaderCreateInfo& create_info, utl::vector<FShaderCreationBlockInfo>&& shader_stages)
{
	m_shaderId = m_pRenderContext->CreateShader(name, create_info, std::move(shader_stages));
	BuildReflection();
}

void CShaderObject::Create(const std::string& name, const FComputeShaderCreateInfo& create_info, utl::vector<FShaderCreationBlockInfo>&& shader_stages)
{
	m_shaderId = m_pRenderContext->CreateShader(name, create_info, std::move(shader_stages));
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

	m_pRenderContext->UpdateShaderResources(update_data);
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

	m_pRenderContext->FlushConstantRanges(m_shaderId, constants, queue);
}

void CShaderObject::Bind(EQueueTypeFlagBits queue)
{
	m_pRenderContext->BindShader(m_shaderId, queue);
}

void CShaderObject::BindResources(EQueueTypeFlagBits queue)
{
	m_pRenderContext->BindShaderResources(m_shaderId, queue);
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
	m_reflection = m_pRenderContext->GetShaderReflection(m_shaderId);

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