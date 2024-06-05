#include "ShaderObject.h"

using namespace engine;
using namespace engine::render;

CShaderObject::CShaderObject(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

CShaderObject::~CShaderObject()
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->DestroyShader(m_resourceId);
}

void CShaderObject::Create(const std::string& name, const FGraphicsShaderCreateInfo& create_info, const utl::vector<FShaderCreationBlockInfo>& stages)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	m_resourceId = render_context->CreateShader(name, create_info, stages);
	BuildReflection();
}

void CShaderObject::Create(const std::string& name, const FComputeShaderCreateInfo& create_info, const utl::vector<FShaderCreationBlockInfo>& stages)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	m_resourceId = render_context->CreateShader(name, create_info, stages);
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

void CShaderObject::BindBuffer(u64 name, u64 buffer_resource)
{
	auto& resource_manager = m_pRenderInterface->GetResourceManager();
	auto& buffer = resource_manager->GetBufferObject(buffer_resource);
	m_bindBuffers[name] = buffer->Handle();
}

void CShaderObject::BindTexture(u64 name, u64 texture_resource, u32 mip)
{
	auto& resource_manager = m_pRenderInterface->GetResourceManager();
	auto& texture = resource_manager->GetTextureObject(texture_resource);
	m_bindTextures[name] = FTextureBindInfo{ texture->Handle(), mip };
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
	update_data.shaderId = m_resourceId;
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
	render_context->FlushConstantRanges(m_resourceId, constants, queue);
}

void CShaderObject::Bind(EQueueTypeFlagBits queue)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->BindShader(m_resourceId, queue);
}

void CShaderObject::BindResources(EQueueTypeFlagBits queue)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	render_context->BindShaderResources(m_resourceId, queue);
}

const FShaderReflectionData& CShaderObject::GetReflection() const
{
	return m_reflection;
}

void CShaderObject::BuildReflection()
{
	auto& render_context = m_pRenderInterface->GetRenderContext();
	m_reflection = render_context->GetShaderReflection(m_resourceId);

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