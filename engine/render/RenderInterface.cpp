#include "RenderInterface.h"
#include <uuid.hpp>

#include <engine/render/render_api/vulkan/VkRenderBackend.h>
#include <engine/render/render_api/vulkan/Buffer.h>
#include <engine/render/render_api/vulkan/Sampler.h>
#include <engine/render/render_api/vulkan/Image.h>
#include <engine/render/render_api/vulkan/ImageView.h>
#include <engine/render/render_api/vulkan/Texture.h>

#include "BufferObject.h"
#include "ShaderObject.h"
#include "TextureObject.h"
#include "VertexHandler.h"

#include "RenderWorld.h"
#include "ResourceLoader.h"
#include "render_graph/RenderGraph.h"

#include <shader_compiler/compiler_util.h>

#include <engine/filesystem/vfs_helper.h>

using namespace engine;
using namespace engine::render;

constexpr const u64 MAX_RENDER_OBJECT_COUNT{ 16384ull };
constexpr const u64 MAX_PRIMITIVE_COUNT{ 512ull };
constexpr const u64 MAX_MATERIAL_COUNT{ 512ull };

std::unique_ptr<IRenderBackend> make_render_context(const FRenderInterfaceCreateInfo& rhi_ci)
{
	FRenderContextCreateInfo context_ci{};
	std::unique_ptr<IRenderBackend> render_backend{};

	if (rhi_ci.render_api == ERenderApi::eVulkan10 || rhi_ci.render_api == ERenderApi::eVulkan11 ||
		rhi_ci.render_api == ERenderApi::eVulkan12 || rhi_ci.render_api == ERenderApi::eVulkan13)
	{
		render_backend = std::make_unique<vulkan::VkRenderBackend>();

		context_ci.apiVersion = rhi_ci.render_api;
		context_ci.requiredDeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
			VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
			VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
			VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME,
			VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME,
			VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
			VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
			VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME,
			VK_KHR_8BIT_STORAGE_EXTENSION_NAME,
			VK_KHR_16BIT_STORAGE_EXTENSION_NAME,
			VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME,
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
			VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME,
			VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
			VK_KHR_SPIRV_1_4_EXTENSION_NAME,
			//VK_EXT_MESH_SHADER_EXTENSION_NAME,
			VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME,
			//VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
			VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME,
			//VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME,
			VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME
		};
		
		context_ci.optionalDeviceExtensions = {
			VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME
		};

#ifndef BUILD_WITHOUT_DEBUGGIND
		context_ci.validationFeatures = vulkan::SYNCHRONIZATION_VALIDATION_FLAG_BIT;
#endif

		context_ci.window = rhi_ci.window;

		render_backend->Create(context_ci);
	}

	return render_backend;
}

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

CRenderInterface::CRenderInterface(filesystem::IVirtualFileSystemInterface* vfs)
{
	m_pVFS = vfs;

	m_pResourceLoader = std::make_unique<CResourceLoader>();
	m_pRenderPacket = std::make_unique<FRenderDataPacket>();

	render_resources = std::make_unique<resource_container_t>();

	m_pRenderGraph = std::make_unique<CRenderGraph>(this);
	m_pRenderWorld = std::make_unique<CRenderWorld>(this);
}

CRenderInterface::~CRenderInterface()
{

}

void CRenderInterface::PreInitialize(const FRenderInterfaceCreateInfo& rhi_ci)
{
	m_pResourceLoader->Initialize(m_pVFS);

	m_pRenderContext = make_render_context(rhi_ci);

	FGlobalUniformCreateInfo global_buffer_ci{};
	global_buffer_ci.uniforms[shader_bindings::global_uniform_frame_data] = sizeof(FGlobalUniformData);
	global_buffer_ci.uniforms[shader_bindings::global_buffer_references] = sizeof(FGlobalBufferReference);
	global_buffer_ci.uniforms[shader_bindings::global_post_process_data] = sizeof(FGlobalPostProcessingData);
	m_pRenderContext->CreateGlobalDescriptor(global_buffer_ci);

	m_pRenderWorld->Create();
}

void CRenderInterface::Initialize()
{
	utl::vector<uint8_t> render_config{};
	m_pVFS->ReadFile("/root/embed/rendering/render_config.xml", render_config);
	utl::vector<uint8_t> render_config_schema{};
	m_pVFS->ReadFile("/root/embed/rendering/render_config.xsd", render_config_schema);
	m_pRenderGraph->Create(render_config, render_config_schema);
}

void CRenderInterface::PostInitialize()
{
	m_pRenderGraph->Build();
	m_pRenderGraph->PostLoad();
}

// There should already be information about geometry here, and everything that needed to be updated should be updated.
void CRenderInterface::PreUpdate(f32 dt)
{
	
}

// Updates global unoforms. It also acalled after another update
void CRenderInterface::Update(f32 dt)
{
	m_pRenderGraph->Update(dt);
	m_pRenderWorld->Begin(dt);

	const auto& world_stats = m_pRenderWorld->GetStats();

	FGlobalUniformData global_uniform{};
	global_uniform.dt = dt;
	global_uniform.directionalLightCount = utl::cast<u32>(world_stats.directionalLightCount);
	global_uniform.pointLightCount = utl::cast<u32>(world_stats.pointLightCount);
	global_uniform.spotLightCount = utl::cast<u32>(world_stats.spotLightCount);
	global_uniform.lightCount = utl::cast<u32>(world_stats.lightCount);
	global_uniform.cullerCount = utl::cast<u32>(world_stats.cullerCount);
	global_uniform.cameraCount = utl::cast<u32>(world_stats.cullerCount);
	global_uniform.maxCommandsCount = utl::cast<u32>(world_stats.maxDrawCommands);
	global_uniform.rawCommandsCount = utl::cast<u32>(world_stats.drawCommandCount);
	global_uniform.defaultCameraID = utl::cast<u32>(m_pRenderWorld->GetDefaultCameraID());
	global_uniform.frameNumber = m_frameCounter;

	FGlobalUniformUpdateInfo update_info{};
	auto& frame_gata_uniform = update_info.uniforms[shader_bindings::global_uniform_frame_data];
	frame_gata_uniform.data = &global_uniform;
	frame_gata_uniform.size = sizeof(FGlobalUniformData);
	frame_gata_uniform.binding = 0u;

	FGlobalBufferReference global_references{};
	global_references.nodeBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eNodeBuffer)->Buffer()->GetDescription();
	global_references.matrixBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eMatricesBuffer)->Buffer()->GetDescription();
	global_references.materialBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eMaterialBuffer)->Buffer()->GetDescription();
	global_references.meshBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eMeshBuffer)->Buffer()->GetDescription();

	global_references.lightsBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eLightBuffer)->Buffer()->GetDescription();

	global_references.cascadedShadowBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eCascadedShadowBuffer)->Buffer()->GetDescription();
	global_references.omniShadowBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eOmniShadowBuffer)->Buffer()->GetDescription();
	global_references.spotShadowBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eSpotShadowBuffer)->Buffer()->GetDescription();

	global_references.drawInfoBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eDrawMeshInfoBuffer)->Buffer()->GetDescription();
	global_references.rawDrawInfoBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eRawDrawMeshInfoBuffer)->Buffer()->GetDescription();
	global_references.clusterBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eClusterInfoBuffer)->Buffer()->GetDescription();
	global_references.frustumSidesBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eFrustumBuffer)->Buffer()->GetDescription();

	global_references.indirectCommandBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eDrawCommandBuffer)->Buffer()->GetDescription();
	global_references.indirectCountBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eCountBuffer)->Buffer()->GetDescription();
	global_references.indirectTaskCommandBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eMeshCommandBuffer)->Buffer()->GetDescription();

	global_references.textureRemapBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eTextureRemapBuffer)->Buffer()->GetDescription();
	global_references.cameraBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eCameraBuffer)->Buffer()->GetDescription();

	auto& buffer_ref_uniform = update_info.uniforms[shader_bindings::global_buffer_references];
	buffer_ref_uniform.data = &global_references;
	buffer_ref_uniform.size = sizeof(FGlobalBufferReference);
	buffer_ref_uniform.binding = 1u;

	auto& post_process = m_pRenderWorld->GetPostProcessingData();

	FGlobalPostProcessingData postprocessing{};
	// FXAA
	postprocessing.enable_fxaa = utl::cast<i32>(post_process.fxaa.enable);
	postprocessing.fxaa_quality = fxaa_quality_variant[post_process.fxaa.quality];
	postprocessing.fxaa_threshold = fxaa_threshold_variant[post_process.fxaa.threshold];
	postprocessing.fxaa_threshold_min = fxaa_threshold_min_variant[post_process.fxaa.threshold_min];
	// Tonemapping
	postprocessing.enable_tonemap = utl::cast<i32>(post_process.tonemap.enable);
	postprocessing.tm_gamma = post_process.tonemap.gamma;
	postprocessing.tm_exposure = post_process.tonemap.exposure;
	postprocessing.tm_white_point = post_process.tonemap.whitePoint;
	postprocessing.tm_adaptive = utl::cast<i32>(post_process.tonemap.adaptive);
	// Bloom
	postprocessing.enable_bloom = utl::cast<i32>(post_process.bloom.enable);
	postprocessing.b_bloom_strength = post_process.bloom.strength;
	postprocessing.b_threshold = post_process.bloom.threshold;
	postprocessing.b_filter_radius = post_process.bloom.filter_radius;
	// Dof
	postprocessing.enable_dof = utl::cast<i32>(post_process.dof.enable);
	postprocessing.df_sample_count = post_process.dof.bokeh_samples;
	postprocessing.df_poly = post_process.dof.bokeh_poly;
	postprocessing.df_focus_point = post_process.dof.focus_point;
	postprocessing.df_near_field = post_process.dof.near_field;
	postprocessing.df_near_transition = post_process.dof.near_transition;
	postprocessing.df_far_field = post_process.dof.far_field;
	postprocessing.df_far_transition = post_process.dof.far_transition;
	// Chromatic aberration
	postprocessing.enable_chromatic_aberration = utl::cast<i32>(post_process.aberration.enable);
	postprocessing.ca_distortion = post_process.aberration.distortion;
	postprocessing.ca_iterations = post_process.aberration.iterations;
	// Film grain
	postprocessing.enable_film_grain = utl::cast<i32>(post_process.filmgrain.enable);
	postprocessing.fg_amount = post_process.filmgrain.amount;
	// Vignette
	postprocessing.enable_vignette = utl::cast<i32>(post_process.vignette.enable);
	postprocessing.v_inner_radius = post_process.vignette.inner;
	postprocessing.v_outer_radius = post_process.vignette.outer;
	postprocessing.v_opacity = post_process.vignette.opacity;
	// Motion blur
	postprocessing.enable_motion_blur = utl::cast<i32>(post_process.motionblur.enable);
	postprocessing.mb_scale = (dt / 0.01666666f) * post_process.motionblur.scele;
	postprocessing.mb_max_samples = post_process.motionblur.max_samples;
	postprocessing.mb_max_velocity = post_process.motionblur.max_velocity;
	postprocessing.mb_threshold = post_process.motionblur.threshold;

	auto& postprocessing_uniform = update_info.uniforms[shader_bindings::global_post_process_data];
	postprocessing_uniform.data = &postprocessing;
	postprocessing_uniform.size = sizeof(FGlobalPostProcessingData);
	postprocessing_uniform.binding = 2u;

	m_pRenderContext->UpdateGlobalDescriptorData(update_info);
}

// Calls before another post updates
void CRenderInterface::PostUpdate(f32 dt)
{

}

// Calls after another PreRender
void CRenderInterface::PreRender()
{
	m_pRenderContext->BeginFrame();
}

// Calls after another Render
void CRenderInterface::Render()
{	
	m_pRenderGraph->Execute();
}

// Calls before another PreRender
void CRenderInterface::PostRender()
{
	m_pRenderContext->EndFrame();
	m_pRenderWorld->End();
	++m_frameCounter;
}

FRHIHandle CRenderInterface::CreateTextureObject(const std::string& path)
{
	auto name = filesystem::fs::get_stem(path);
	auto ext = filesystem::fs::get_ext(path);
	auto ext_hash = utl::murmur_hash(ext);
	auto mime = utl::cast<EMimeType>(ext_hash);

	FRHIHandle texture_id = FRHIHandle(render_resources->getId<ITextureObject>(name.c_str()));
	if (texture_id)
		return texture_id;

	utl::vector<uint8_t> data{};
	m_pVFS->ReadFile(path, data);

	FTextureCreateInfo create_info; 
	FTextureUploadInfo upload_info;
	FTextureResourceHandle resource_handle;
	m_pResourceLoader->LoadTexture(data, mime, create_info, upload_info, resource_handle);

	texture_id = CreateTextureObject(name, create_info, upload_info);
	m_pResourceLoader->DestroyHandle(resource_handle);

	return texture_id;
}

FRHIHandle CRenderInterface::CreateTextureObject(const std::string& name, const FTextureCreateInfo& texture_ci)
{
	auto new_texture = std::make_unique<CTextureObject>(this);
	new_texture->Create(name, ApplyTextureExtent(texture_ci));

	auto handle = FRHIHandle(render_resources->add<ITextureObject>(name.c_str(), std::move(new_texture)));
	auto& texture = GetTextureObject(handle);

	FRenderResource resource_desc{};
	resource_desc.handle = texture->Handle();

	if (texture_ci.flags & ETextureCreateFlagBits::eRenderTarget)
		resource_desc.type = (texture_ci.flags & ETextureCreateFlagBits::eStorage) ? ERenderResourceTypeFlagBits::eStorageTarget : ERenderResourceTypeFlagBits::eRenderTarget;
	else
		resource_desc.type = ERenderResourceTypeFlagBits::eStaticImage;

	m_pRenderGraph->AddRenderResource(name, resource_desc);

	return handle;
}

FRHIHandle CRenderInterface::CreateTextureObject(const std::string& name, const FTextureCreateInfo& texture_ci, const FTextureUploadInfo& upload_info)
{
	auto new_texture = std::make_unique<CTextureObject>(this);
	new_texture->Create(name, ApplyTextureExtent(texture_ci), upload_info);

	auto handle = FRHIHandle(render_resources->add<ITextureObject>(name.c_str(), std::move(new_texture)));
	auto& texture = GetTextureObject(handle);

	FRenderResource resource_desc{};
	resource_desc.handle = texture->Handle();
	
	if (texture_ci.flags & ETextureCreateFlagBits::eRenderTarget)
		resource_desc.type = (texture_ci.flags & ETextureCreateFlagBits::eStorage) ? ERenderResourceTypeFlagBits::eStorageTarget : ERenderResourceTypeFlagBits::eRenderTarget;
	else
		resource_desc.type = ERenderResourceTypeFlagBits::eStaticImage;

	m_pRenderGraph->AddRenderResource(name, resource_desc);

	return handle;
}

void CRenderInterface::DestroyTextureObject(FRHIHandle handle)
{
	render_resources->remove<ITextureObject>(handle.Get());

	// Delete resource from render graph
	// m_pRenderGraph->RemoveRenderResource(???);
}

const std::unique_ptr<ITextureObject>& CRenderInterface::GetTextureObject(FRHIHandle handle)
{
	return render_resources->get<ITextureObject>(handle.Get());
}

const std::unique_ptr<ITextureObject>& CRenderInterface::GetTextureObject(const char* name)
{
	return render_resources->get<ITextureObject>(name);
}


FRHIHandle CRenderInterface::CreateBufferObject(const std::string& name, const FBufferCreateInfo& buffer_ci)
{
	auto new_buffer = std::make_unique<CBufferObject>(this);
	new_buffer->Create(name, buffer_ci);

	auto handle = FRHIHandle(render_resources->add<IBufferObject>(name.c_str(), std::move(new_buffer)));
	auto& buffer = GetBufferObject(handle);

	FRenderResource resource_desc{};
	resource_desc.handle = buffer->Handle();
	resource_desc.type = ERenderResourceTypeFlagBits::eBuffer;

	m_pRenderGraph->AddRenderResource(name, resource_desc);
	return handle;
}

void CRenderInterface::DestroyBufferObject(FRHIHandle handle)
{
	return render_resources->remove<IBufferObject>(handle.Get());
}

const std::unique_ptr<IBufferObject>& CRenderInterface::GetBufferObject(FRHIHandle handle)
{
	return render_resources->get<IBufferObject>(handle.Get());
}

const std::unique_ptr<IBufferObject>& CRenderInterface::GetBufferObject(const char* name)
{
	return render_resources->get<IBufferObject>(name);
}

FRHIHandle CRenderInterface::CreateShaderObject(const std::string& name, const FGraphicsShaderCreateInfo& shader_ci)
{
	utl::vector<FShaderCreationBlockInfo> shader_stages{};
	if (!m_pResourceLoader->LoadShaderData(GetShaderNameTag(name), shader_stages))
	{
		log_error("Shader with name: \"{}\" was not found.", name);
		return FRHIHandle{};
	}

	auto new_shader = std::make_unique<CShaderObject>(this);
	new_shader->Create(name, shader_ci, std::move(shader_stages));

	auto uuid = utl::to_string(utl::generate_guid());
	return FRHIHandle(render_resources->add<IShaderObject>(uuid.c_str(), std::move(new_shader)));
}

FRHIHandle CRenderInterface::CreateShaderObject(const std::string& name, const FComputeShaderCreateInfo& shader_ci)
{
	utl::vector<FShaderCreationBlockInfo> shader_stages{};
	if (!m_pResourceLoader->LoadShaderData(GetShaderNameTag(name), shader_stages))
	{
		log_error("Shader with name: \"{}\" was not found.", name);
		return FRHIHandle{};
	}

	auto new_shader = std::make_unique<CShaderObject>(this);
	new_shader->Create(name, shader_ci, std::move(shader_stages));

	auto uuid = utl::to_string(utl::generate_guid());
	return FRHIHandle(render_resources->add<IShaderObject>(uuid.c_str(), std::move(new_shader)));
}

void CRenderInterface::DestroyShaderObject(FRHIHandle handle)
{
	return render_resources->remove<IShaderObject>(handle.Get());
}

const std::unique_ptr<IShaderObject>& CRenderInterface::GetShaderObject(FRHIHandle handle)
{
	return render_resources->get<IShaderObject>(handle.Get());
}

const std::unique_ptr<IShaderObject>& CRenderInterface::GetShaderObject(const char* name)
{
	return render_resources->get<IShaderObject>(name);
}


FRHIHandle CRenderInterface::CreateVertexHandler(const std::string& name, const FVertexHandlerCreateInfo& handler_ci)
{
	auto vertex_handler = std::make_unique<CVertexHandler>(this);
	vertex_handler->Create(handler_ci);

	return FRHIHandle(render_resources->add<IVertexHandler>(name.c_str(), std::move(vertex_handler)));
}

void CRenderInterface::DestroyVertexHandler(FRHIHandle handle)
{

}

const std::unique_ptr<IVertexHandler>& CRenderInterface::GetVertexHandler(FRHIHandle id)
{
	return render_resources->get<IVertexHandler>(id.Get());
}

void CRenderInterface::UpdateSurfaceSize(i32 width, i32 height)
{
	m_pRenderContext->UpdateSurface(width, height);
}

const std::unique_ptr<FRenderDataPacket>& CRenderInterface::GetRenderPacket()
{
	return m_pRenderPacket;
}

const std::unique_ptr<IRenderGraph>& CRenderInterface::GetRenderGraph()
{
	return m_pRenderGraph;
}

const std::unique_ptr<IRenderWorld>& CRenderInterface::GetRenderWorld()
{
	return m_pRenderWorld;
}

const std::unique_ptr<IRenderBackend>& CRenderInterface::GetRenderBackend()
{
	return m_pRenderContext;
}

const std::unique_ptr<IResourceLoader>& CRenderInterface::GetResourceLoader()
{
	return m_pResourceLoader;
}

FTextureCreateInfo CRenderInterface::ApplyTextureExtent(const FTextureCreateInfo& texture_ci)
{
	auto render_extent = m_pRenderContext->GetRenderResolution();

	FTextureCreateInfo texture_create_info{ texture_ci };
	if (texture_create_info.size.policy == ERenderTargetSizePolicy::eFull)
	{
		texture_create_info.size.width = render_extent.x;
		texture_create_info.size.height = render_extent.y;
	}
	else if (texture_create_info.size.policy == ERenderTargetSizePolicy::eScalable)
	{
		texture_create_info.size.width = utl::cast<u32>(utl::cast<f32>(render_extent.x) * texture_create_info.size.scale);
		texture_create_info.size.height = utl::cast<u32>(utl::cast<f32>(render_extent.y) * texture_create_info.size.scale);
	}

	return texture_create_info;
}

FShaderUploadInfo CRenderInterface::GetShaderNameTag(const std::string& name)
{
	//auto vendor = m_pRenderContext->GetDeviceVendor();
	auto name_info = get_shader_name_info(name);

	//switch (vendor)
	//{
	//case engine::render::EAdapterVendor::eNone: break;
	//case engine::render::EAdapterVendor::eNVidia: name_info.definitions.emplace_back("VENDOR_NVIDIA"); break;
	//case engine::render::EAdapterVendor::eAMD: name_info.definitions.emplace_back("VENDOR_AMD"); break;
	//case engine::render::EAdapterVendor::eIntel: name_info.definitions.emplace_back("VENDOR_INTEL"); break;
	//default: break;
	//}

	FShaderUploadInfo load_info{};
	load_info.libraryId = utl::murmur_hash(name_info.library_name);
	load_info.shaderId = shader_compiler::make_module_unique_name(name_info.shader_name, name_info.definitions);

	return load_info;
}