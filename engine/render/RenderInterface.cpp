#include "RenderInterface.h"

#include "render_api/vulkan/VkRenderContext.h"

#include "RenderWorld.h"
#include "VertexHandler.h"
#include "BufferObject.h"
#include "render_graph/RenderGraph.h"

#include <shader_compiler/compiler_util.h>

#include <engine/filesystem/vfs_helper.h>

using namespace engine;
using namespace engine::render;

constexpr const u64 MAX_RENDER_OBJECT_COUNT{ 16384ull };
constexpr const u64 MAX_PRIMITIVE_COUNT{ 512ull };
constexpr const u64 MAX_MATERIAL_COUNT{ 512ull };

void copy_platform_specific_create_data(FRenderContextCreateInfo& context_ci, const FRenderInterfaceCreateInfo& rhi_ci)
{
#if defined(PLATFORM_WIN32)
	context_ci.hinstance = rhi_ci.hinstance;
	context_ci.hwnd = rhi_ci.hwnd;
#endif
}

stl::unique_ptr<IRenderContext> make_render_context(const FRenderInterfaceCreateInfo& rhi_ci)
{
	FRenderContextCreateInfo context_ci{};
	stl::unique_ptr<IRenderContext> render_context{};

	if (rhi_ci.render_api == ERenderApi::eVulkan10 || rhi_ci.render_api == ERenderApi::eVulkan11 ||
		rhi_ci.render_api == ERenderApi::eVulkan12 || rhi_ci.render_api == ERenderApi::eVulkan13)
	{
		render_context = stl::make_unique<vulkan::CVkRenderContext>();

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
			VK_EXT_MESH_SHADER_EXTENSION_NAME,
			//VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
			VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME
		};
		
		context_ci.optionalDeviceExtensions = {
			VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME
		};

#ifndef BUILD_WITHOUT_DEBUGGIND
		context_ci.validationState = true;
		context_ci.validationLayers = { "VK_LAYER_KHRONOS_validation", "VK_LAYER_KHRONOS_synchronization2" };
		context_ci.validationFeatures = vulkan::SYNCHRONIZATION_VALIDATION_FLAG_BIT;
#endif

		context_ci.initialWidth = rhi_ci.initialWidth;
		context_ci.initialHeight = rhi_ci.initialHeight;

		copy_platform_specific_create_data(context_ci, rhi_ci);

		render_context->create(context_ci);
	}

	return render_context;
}

struct FShaderNameInfo
{
	stl::string library_name;
	stl::string shader_name;
	stl::vector<stl::string> definitions;
};

FShaderNameInfo get_shader_name_info(const stl::string& shader_name)
{
	FShaderNameInfo name_info{};

	stl::string token;
	stl::stringstream ss(shader_name);

	if (stl::getline(ss, token, ':'))
		name_info.library_name = token;

	if (stl::getline(ss, token, ':'))
		name_info.shader_name = token;

	while (stl::getline(ss, token, ':'))
		name_info.definitions.emplace_back(token);

	return name_info;
}

CRenderInterface::CRenderInterface(filesystem::IVirtualFileSystemInterface* vfs)
{
	m_pVFS = vfs;

	m_pResourceLoader = stl::make_unique<CResourceLoader>();
	m_pRenderScriptCore = stl::make_unique<CRenderScriptCore>();
	m_pRenderPacket = stl::make_unique<FRenderDataPacket>();

	m_pResources = stl::make_unique<CResourceContainer>();
	m_pResources->registerResource<IVertexHandler>();

	m_pRenderGraph = stl::make_unique<CRenderGraph>(this);
	m_pRenderWorld = stl::make_unique<CRenderWorld>(this);
}

CRenderInterface::~CRenderInterface()
{

}

void CRenderInterface::create(const FRenderInterfaceCreateInfo& rhi_ci)
{
	m_pResourceLoader->initialize(m_pVFS);

	m_pRenderContext = make_render_context(rhi_ci);

	FGlobalUniformCreateInfo global_buffer_ci{};
	global_buffer_ci.uniforms[utl::utils::fnv1a_64_hash("GlobalUniformFrameData")] = sizeof(FGlobalUniformData);
	global_buffer_ci.uniforms[utl::utils::fnv1a_64_hash("GlobalBufferReferences")] = sizeof(FGlobalBufferReference);
	m_pRenderContext->createGlobalDescriptor(global_buffer_ci);

	m_pRenderWorld->create();
	//m_pRenderGraph->create();

	m_pRenderScriptCore->create(this);

	stl::vector<uint8_t> render_config{};
	m_pVFS->readFile("/root/embed/rendering/test_render_config.lua", render_config);
	m_pRenderScriptCore->build("render_config", render_config);

	m_pRenderGraph->build();
}

void CRenderInterface::render(f32 dt)
{
	m_pRenderContext->beginFrame();

	m_pRenderGraph->update(dt);
	m_pRenderWorld->render(dt);

	const auto& world_stats = m_pRenderWorld->getStats();

	FGlobalUniformData global_uniform{};
	global_uniform.view = m_pRenderPacket->camera.view;
	global_uniform.invView = m_pRenderPacket->camera.invView;
	global_uniform.viewportDimension = m_pRenderPacket->camera.dimensions;
	global_uniform.invViewportDimension = 1.f / m_pRenderPacket->camera.dimensions;
	global_uniform.nearPlane = m_pRenderPacket->camera.nearp;
	global_uniform.farPlane = m_pRenderPacket->camera.farp;
	global_uniform.directionalLightCount = utl::cast<u32>(world_stats.directionalLightCount);
	global_uniform.pointLightCount = utl::cast<u32>(world_stats.pointLightCount);
	global_uniform.spotLightCount = utl::cast<u32>(world_stats.spotLightCount);
	global_uniform.rawCommandsCount = utl::cast<u32>(world_stats.drawCommandCount);
	global_uniform.defaultViewport = utl::cast<u32>(m_pRenderGraph->getDefaultViewportIndex());

	FGlobalUniformUpdateInfo update_info{};
	auto& frame_gata_uniform = update_info.uniforms[utl::utils::fnv1a_64_hash("GlobalUniformFrameData")];
	frame_gata_uniform.data = &global_uniform;
	frame_gata_uniform.size = sizeof(FGlobalUniformData);
	frame_gata_uniform.binding = 0u;

	FGlobalBufferReference global_references{};
	global_references.nodeBuffer = m_pRenderWorld->getDrawBuffer(ERenderBufferIndices::eNodeBuffer)->buffer()->address();
	global_references.jointsBuffer = m_pRenderWorld->getDrawBuffer(ERenderBufferIndices::eSkinBuffer)->buffer()->address();
	global_references.materialBuffer = m_pRenderWorld->getDrawBuffer(ERenderBufferIndices::eMaterialBuffer)->buffer()->address();
	global_references.meshBuffer = m_pRenderWorld->getDrawBuffer(ERenderBufferIndices::eMeshBuffer)->buffer()->address();

	global_references.directionalLightBuffer = m_pRenderWorld->getDrawBuffer(ERenderBufferIndices::eDirectionalLightBuffer)->buffer()->address();
	global_references.pointLightBuffer = m_pRenderWorld->getDrawBuffer(ERenderBufferIndices::ePointLightBuffer)->buffer()->address();
	global_references.spotLightBuffer = m_pRenderWorld->getDrawBuffer(ERenderBufferIndices::eSpotLightBuffer)->buffer()->address();

	global_references.cascadedShadowBuffer = m_pRenderWorld->getDrawBuffer(ERenderBufferIndices::eCascadedShadowBuffer)->buffer()->address();
	global_references.omniShadowBuffer = m_pRenderWorld->getDrawBuffer(ERenderBufferIndices::eOmniShadowBuffer)->buffer()->address();
	global_references.spotShadowBuffer = m_pRenderWorld->getDrawBuffer(ERenderBufferIndices::eSpotShadowBuffer)->buffer()->address();

	global_references.drawInfoBuffer = m_pRenderWorld->getDrawBuffer(ERenderBufferIndices::eDrawMeshInfoBuffer)->buffer()->address();
	global_references.clusterBuffer = m_pRenderWorld->getDrawBuffer(ERenderBufferIndices::eClusterInfoBuffer)->buffer()->address();

	global_references.indirectCommandBuffer = m_pRenderWorld->getDrawBuffer(ERenderBufferIndices::eDrawCommandBuffer)->buffer()->address();
	global_references.indirectTaskCommandBuffer = m_pRenderWorld->getDrawBuffer(ERenderBufferIndices::eMeshCommandBuffer)->buffer()->address();

	global_references.textureRemapBuffer = m_pRenderWorld->getDrawBuffer(ERenderBufferIndices::eTextureRemapBuffer)->buffer()->address();
	global_references.viewportBuffer = m_pRenderWorld->getDrawBuffer(ERenderBufferIndices::eViewportBuffer)->buffer()->address();

	auto& buffer_ref_uniform = update_info.uniforms[utl::utils::fnv1a_64_hash("GlobalBufferReferences")];
	buffer_ref_uniform.data = &global_references;
	buffer_ref_uniform.size = sizeof(FGlobalBufferReference);
	buffer_ref_uniform.binding = 1u;

	m_pRenderContext->updateGlobalDescriptorData(update_info);
	m_pRenderGraph->execute(dt);

	m_pRenderContext->endFrame();
}

u64 CRenderInterface::createTexture(const stl::string& path)
{
	auto name = filesystem::fs::get_stem(path);
	return m_pRenderGraph->addStaticResource(name, path);
}

u64 CRenderInterface::loadTexture(const stl::string& path, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info)
{
	auto name = filesystem::fs::get_stem(path);
	auto ext = filesystem::fs::get_ext(path);
	auto ext_hash = utl::utils::const_hash(ext.c_str(), ext.size());
	auto mime = utl::cast<EMimeType>(ext_hash);

	u64 texture_id = m_pRenderContext->getTextureId(name);
	if (texture_id != ~0ull)
		return texture_id;

	stl::vector<uint8_t> data{};
	m_pVFS->readFile(path, data);

	m_pResourceLoader->loadTexture(data, mime, create_info, upload_info);

	return ~0ull;
}

u64 CRenderInterface::createShader(const stl::string& name, const FGraphicsShaderCreateInfo& create_info)
{
	auto name_info = getShaderNameTag(name);
	return createShader<FGraphicsShaderCreateInfo>(name_info, create_info);
}

u64 CRenderInterface::createShader(const stl::string& name, const FComputeShaderCreateInfo& create_info)
{
	auto name_info = getShaderNameTag(name);
	return createShader<FComputeShaderCreateInfo>(name_info, create_info);
}

u64 CRenderInterface::createVertexHandler(const stl::string& name, const FVertexHandlerCreateInfo& handler_ci)
{
	auto vertex_handler = stl::make_unique<CVertexHandler>(this);
	vertex_handler->create(handler_ci);

	return m_pResources->add<IVertexHandler>(name.c_str(), stl::move(vertex_handler));
}

const stl::unique_ptr<IVertexHandler>& CRenderInterface::getVertexHandler(u64 id)
{
	return m_pResources->get<IVertexHandler>(id);
}

void CRenderInterface::updateSurfaceSize(i32 width, i32 height)
{
	m_pRenderContext->updateSurface(width, height);
}

const stl::unique_ptr<FRenderDataPacket>& CRenderInterface::getRenderPacket()
{
	return m_pRenderPacket;
}

const stl::unique_ptr<IRenderGraph>& CRenderInterface::getRenderGraph()
{
	return m_pRenderGraph;
}

const stl::unique_ptr<IRenderWorld>& CRenderInterface::getRenderWorld()
{
	return m_pRenderWorld;
}

const stl::unique_ptr<IRenderContext>& CRenderInterface::getRenderContext()
{
	return m_pRenderContext;
}

FShaderUploadInfo CRenderInterface::getShaderNameTag(const stl::string& name)
{
	auto name_info = get_shader_name_info(name);

	FShaderUploadInfo load_info{};
	load_info.libraryId = utl::utils::fnv1a_64_hash(name_info.library_name.c_str());
	load_info.shaderId = shader_compiler::make_module_unique_name(name_info.shader_name, name_info.definitions);
	load_info.originalName = name;

	return load_info;
}