#include "ResourceManager.h"

#include <shader_compiler/compiler_util.h>

#include "ResourceLoader.h"
#include "BufferObject.h"
#include "ShaderObject.h"
#include "TextureObject.h"
#include "VertexHandler.h"

#include <engine/filesystem/vfs_helper.h>

using namespace engine::render;

struct FShaderNameInfo
{
	std::string library_name;
	std::string shader_name;
	utl::vector<std::string> definitions;
};

FShaderNameInfo get_shader_name_info(const std::string& shader_name)
{
	FShaderNameInfo name_info{};

	std::string token;
	std::stringstream ss(shader_name);

	if (std::getline(ss, token, ':'))
		name_info.library_name = token;

	if (std::getline(ss, token, ':'))
		name_info.shader_name = token;

	while (std::getline(ss, token, ':'))
		name_info.definitions.emplace_back(token);

	return name_info;
}

FShaderUploadInfo get_shader_name_tag(const std::string& name)
{
	//auto vendor = m_pRenderContext->GetDeviceVendor();
	auto name_info = get_shader_name_info(name);

	FShaderUploadInfo load_info{};
	load_info.libraryId = utl::fnv1a_32_hash_cstr(name_info.library_name.c_str());
	load_info.shaderId = shader_compiler::make_module_unique_name(name_info.shader_name, name_info.definitions);

	return load_info;
}



CRenderResourceManager::CRenderResourceManager(IRenderInterface* render_interface, filesystem::IVirtualFileSystemInterface* vfs)
{
	m_pVFS = vfs;
	m_pRenderInterface = render_interface;

	m_pResources = std::make_unique<resource_container_t>();
	m_pResourceLoader = std::make_unique<CResourceLoader>(vfs);
}

void CRenderResourceManager::Create()
{
	m_pResourceLoader->Create("/root/embed/rendering/shader_cache.shc");
}

u64 CRenderResourceManager::CreateTexture(const std::string& name, const FTextureCreateInfo& create_info)
{
	auto new_texture = std::make_unique<CTextureObject>(m_pRenderInterface);
	new_texture->Create(name, create_info);
	return m_pResources->Add<ITextureObject>(name.c_str(), std::move(new_texture));
}

u64 CRenderResourceManager::LoadTexture(const std::string& path)
{
	auto name = filesystem::fs::get_stem(path);
	auto ext = filesystem::fs::get_ext(path);
	auto ext_hash = utl::const_hash(ext.c_str(), ext.size());
	auto mime = utl::cast<EMimeType>(ext_hash);

	u64 texture_id = m_pResources->GetId<ITextureObject>(name.c_str());
	if (texture_id != INVALID_HANDLE)
		return texture_id;

	utl::vector<uint8_t> data{};
	m_pVFS->ReadFile(path, data);

	FTextureCreateInfo create_info;
	FTextureUploadInfo upload_info;
	m_pResourceLoader->LoadTexture(data, mime, create_info, upload_info);

	auto new_texture = std::make_unique<CTextureObject>(m_pRenderInterface);
	new_texture->Create(name, create_info, std::move(upload_info));
	return m_pResources->Add<ITextureObject>(name.c_str(), std::move(new_texture));
}

const std::unique_ptr<ITextureObject>& CRenderResourceManager::GetTextureObject(u64 id)
{
	return m_pResources->Get<ITextureObject>(id);
}

u64 CRenderResourceManager::CreateShader(const std::string& name, const FGraphicsShaderCreateInfo& create_info)
{
	utl::vector<FShaderCreationBlockInfo> shader_stages{};
	LoadShaderStages(name, shader_stages);

	auto new_shader = std::make_unique<CShaderObject>(m_pRenderInterface);
	new_shader->Create(name, create_info, shader_stages);
	return m_pResources->Add<IShaderObject>(name.c_str(), std::move(new_shader));
}

u64 CRenderResourceManager::CreateShader(const std::string& name, const FComputeShaderCreateInfo& create_info) 
{
	utl::vector<FShaderCreationBlockInfo> shader_stages{};
	LoadShaderStages(name, shader_stages);

	auto new_shader = std::make_unique<CShaderObject>(m_pRenderInterface);
	new_shader->Create(name, create_info, shader_stages);
	return m_pResources->Add<IShaderObject>(name.c_str(), std::move(new_shader));
}

const std::unique_ptr<IShaderObject>& CRenderResourceManager::GetShaderObject(u64 id) 
{
	return m_pResources->Get<IShaderObject>(id);
}


u64 CRenderResourceManager::CreateBuffer(const std::string& name, const FBufferCreateInfo& create_info) 
{
	auto new_buffer = std::make_unique<CBufferObject>(m_pRenderInterface);
	new_buffer->Create(name, create_info);
	return m_pResources->Add<IBufferObject>(name.c_str(), std::move(new_buffer));
}

const std::unique_ptr<IBufferObject>& CRenderResourceManager::GetBufferObject(u64 id) 
{
	return m_pResources->Get<IBufferObject>(id);
}

void CRenderResourceManager::LoadShaderStages(const std::string& name, utl::vector<FShaderCreationBlockInfo>& stages)
{
	auto name_info = get_shader_name_tag(name);
	if (m_pResourceLoader->LoadShaderData(name_info, stages))
		return;

	log_error("Shader with name: \"{}\" was not found.", name);
}

u64 CRenderResourceManager::CreateVertexHandler(const std::string& name, const FVertexHandlerCreateInfo& create_info)
{
	auto new_vertex_handler = std::make_unique<CVertexHandler>(m_pRenderInterface);
	new_vertex_handler->Create(create_info);
	return m_pResources->Add<IVertexHandler>(name.c_str(), std::move(new_vertex_handler));
}

const std::unique_ptr<IVertexHandler>& CRenderResourceManager::GetVertexHandler(u64 id)
{
	return m_pResources->Get<IVertexHandler>(id);
}