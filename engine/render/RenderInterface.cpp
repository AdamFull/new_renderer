#include "RenderInterface.h"

#include "render_api/vulkan/VkRenderContext.h"

#include "RenderWorld.h"
#include "VertexHandler.h"
#include "BufferObject.h"
#include "GPUArrayHandle.h"
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

std::unique_ptr<IRenderContext> make_render_context(const FRenderInterfaceCreateInfo& rhi_ci)
{
	FRenderContextCreateInfo context_ci{};
	std::unique_ptr<IRenderContext> render_context{};

	if (rhi_ci.render_api == ERenderApi::eVulkan10 || rhi_ci.render_api == ERenderApi::eVulkan11 ||
		rhi_ci.render_api == ERenderApi::eVulkan12 || rhi_ci.render_api == ERenderApi::eVulkan13)
	{
		render_context = std::make_unique<vulkan::CVkRenderContext>();

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
	std::string library_name;
	std::string shader_name;
	std::vector<std::string> definitions;
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

template<class _Ty>
std::unique_ptr<IGPUArrayHandle> allocateGPUArrayHandle(const std::string& name, u64 count, IRenderInterface* render_interface)
{
	auto gpu_array = std::make_unique<CGPUArrayHandle>(render_interface);
	gpu_array->create(name, sizeof(_Ty), count);
	return gpu_array;
}

CRenderInterface::CRenderInterface(filesystem::IVirtualFileSystemInterface* vfs)
{
	m_pVFS = vfs;

	m_pResourceLoader = std::make_unique<CResourceLoader>();
	m_pRenderScriptCore = std::make_unique<CRenderScriptCore>();
	m_pRenderPacket = std::make_unique<FRenderDataPacket>();

	m_pResources = std::make_unique<CResourceContainer>();
	m_pResources->registerResource<IVertexHandler>();

	m_pRenderGraph = std::make_unique<CRenderGraph>(this);
	m_pRenderWorld = std::make_unique<CRenderWorld>(this);
}

CRenderInterface::~CRenderInterface()
{

}

void CRenderInterface::create(const FRenderInterfaceCreateInfo& rhi_ci)
{
	m_pResourceLoader->initialize(m_pVFS);

	m_pRenderContext = make_render_context(rhi_ci);

	FGlobalUniformCreateInfo global_buffer_ci{};
	global_buffer_ci.uniforms[utl::fnv1a_64_hash("GlobalUniformFrameData")] = sizeof(FGlobalUniformData);
	global_buffer_ci.uniforms[utl::fnv1a_64_hash("GlobalBufferReferences")] = sizeof(FGlobalBufferReference);
	m_pRenderContext->createGlobalDescriptor(global_buffer_ci);

	m_pRenderWorld->create();
	m_pRenderGraph->create();
	m_pRenderGraph->bake();

	m_pRenderScriptCore->create(this);

	//std::vector<uint8_t> render_config{};
	//m_pVFS->readFile("/root/embed/rendering/test_render_config.lua", render_config);
	//m_pRenderScriptCore->build("render_config", render_config);
}

void CRenderInterface::render(f32 dt)
{
	m_pRenderWorld->render(dt);

	m_pRenderContext->beginFrameCommandBuffers();

	const auto& world_stats = m_pRenderWorld->getStats();

	FGlobalUniformData global_uniform{};
	global_uniform.view = m_pRenderPacket->camera.view;
	global_uniform.invView = m_pRenderPacket->camera.invView;
	global_uniform.projection = m_pRenderPacket->camera.projection;
	global_uniform.invProjection = m_pRenderPacket->camera.invProjection;
	global_uniform.viewProjection = m_pRenderPacket->camera.viewProjection;
	global_uniform.invViewProjection = m_pRenderPacket->camera.invViewProjection;
	global_uniform.viewportDimension = m_pRenderPacket->camera.dimensions;
	global_uniform.invViewportDimension = 1.f / m_pRenderPacket->camera.dimensions;
	global_uniform.nearPlane = m_pRenderPacket->camera.nearp;
	global_uniform.farPlane = m_pRenderPacket->camera.farp;
	global_uniform.directionalLightCount = utl::cast<u32>(world_stats.directionalLightCount);
	global_uniform.pointLightCount = utl::cast<u32>(world_stats.pointLightCount);
	global_uniform.spotLightCount = utl::cast<u32>(world_stats.spotLightCount);
	global_uniform.rawCommandsCount = utl::cast<u32>(world_stats.drawCommandCount);

	FGlobalUniformUpdateInfo update_info{};
	auto& frame_gata_uniform = update_info.uniforms[utl::fnv1a_64_hash("GlobalUniformFrameData")];
	frame_gata_uniform.data = &global_uniform;
	frame_gata_uniform.size = sizeof(FGlobalUniformData);
	frame_gata_uniform.binding = 0u;


	auto& buffer_mappings = m_pRenderWorld->getDrawBuffers();
	FGlobalBufferReference global_references{};
	global_references.nodeBuffer = buffer_mappings[node_buffer_index]->address();
	global_references.materialBuffer = buffer_mappings[material_buffer_index]->address();
	global_references.jointsBuffer = buffer_mappings[skin_buffer_index]->address();
	global_references.directionalLightBuffer = buffer_mappings[directional_light_buffer_index]->address();
	global_references.pointLightBuffer = buffer_mappings[point_light_buffer_index]->address();
	global_references.spotLightBuffer = buffer_mappings[spot_light_buffer_index]->address();
	global_references.cascadedShadowBuffer = buffer_mappings[cascaded_shadow_buffer_index]->address();
	global_references.omniShadowBuffer = buffer_mappings[omni_shadow_buffer_index]->address();
	global_references.spotShadowBuffer = buffer_mappings[spot_shadow_buffer_index]->address();
	global_references.batchBuffer = buffer_mappings[draw_batch_buffer_index]->address();
	global_references.indirectCommandBuffer = buffer_mappings[draw_command_buffer_index]->address();

	auto& buffer_ref_uniform = update_info.uniforms[utl::fnv1a_64_hash("GlobalBufferReferences")];
	buffer_ref_uniform.data = &global_references;
	buffer_ref_uniform.size = sizeof(FGlobalBufferReference);
	buffer_ref_uniform.binding = 1u;

	m_pRenderContext->updateGlobalDescriptorData(update_info);
	m_pRenderGraph->execute(dt);
	m_pRenderContext->endFrameCommandBuffers();
}

u64 CRenderInterface::createTexture(const std::string& path)
{
	auto name = filesystem::fs::get_stem(path);
	return m_pRenderGraph->addStaticResource(name, path);
}

u64 CRenderInterface::loadTexture(const std::string& path, FTextureCreateInfo& create_info, FTextureUploadInfo& upload_info)
{
	auto name = filesystem::fs::get_stem(path);
	auto ext = filesystem::fs::get_ext(path);
	auto ext_hash = utl::const_hash(ext.c_str(), ext.size());
	auto mime = utl::cast<EMimeType>(ext_hash);

	u64 texture_id = m_pRenderContext->getTextureId(name);
	if (texture_id != ~0ull)
		return texture_id;

	std::vector<uint8_t> data{};
	m_pVFS->readFile(path, data);

	m_pResourceLoader->loadTexture(data, mime, create_info, upload_info);

	return ~0ull;
}

u64 CRenderInterface::createShader(const std::string& name, const FGraphicsShaderCreateInfo& create_info)
{
	auto name_info = getShaderNameTag(name);
	return createShader<FGraphicsShaderCreateInfo>(name_info, create_info);
}

u64 CRenderInterface::createShader(const std::string& name, const FComputeShaderCreateInfo& create_info)
{
	auto name_info = getShaderNameTag(name);
	return createShader<FComputeShaderCreateInfo>(name_info, create_info);
}

u64 CRenderInterface::createVertexHandler(const std::string& name, const FVertexHandlerCreateInfo& handler_ci)
{
	auto vertex_handler = std::make_unique<CVertexHandler>(this);
	vertex_handler->create(handler_ci);

	return m_pResources->add<IVertexHandler>(name.c_str(), std::move(vertex_handler));
}

const std::unique_ptr<IVertexHandler>& CRenderInterface::getVertexHandler(u64 id)
{
	return m_pResources->get<IVertexHandler>(id);
}

void CRenderInterface::updateSurfaceSize(i32 width, i32 height)
{
	m_pRenderContext->updateSurface(width, height);
}

const std::unique_ptr<FRenderDataPacket>& CRenderInterface::getRenderPacket()
{
	return m_pRenderPacket;
}

const std::unique_ptr<IRenderGraph>& CRenderInterface::getRenderGraph()
{
	return m_pRenderGraph;
}

const std::unique_ptr<IRenderWorld>& CRenderInterface::getRenderWorld()
{
	return m_pRenderWorld;
}

const std::unique_ptr<IRenderContext>& CRenderInterface::getRenderContext()
{
	return m_pRenderContext;
}

FShaderUploadInfo CRenderInterface::getShaderNameTag(const std::string& name)
{
	auto name_info = get_shader_name_info(name);

	FShaderUploadInfo load_info{};
	load_info.libraryId = utl::fnv1a_64_hash(name_info.library_name.c_str());
	load_info.shaderId = shader_compiler::make_module_unique_name(name_info.shader_name, name_info.definitions);
	load_info.originalName = name;

	return load_info;
}