#include "RenderInterface.h"

#include "render_api/vulkan/VkRenderContext.h"
#include "rendering_steps/FullscreenRenderingStep.h"
#include "rendering_steps/RenderScriptRenderingStep.h"
#include "rendering_steps/SemaphoreRenderingStep.h"
#include "rendering_steps/BarrierRenderingStep.h"
#include "rendering_steps/GeometryRenderingStep.h"
#include "rendering_steps/CSMRenderingStep.h"
#include "rendering_steps/ComputeRenderingStep.h"
#include "rendering_steps/ComputeCubemapRenderingStep.h"
#include "rendering_steps/ComputeGeometryRenderingStep.h"
#include "rendering_steps/ComputeCullingRenderingStep.h"

#include "VertexHandler.h"
#include "BufferObject.h"
#include "GPUArrayHandle.h"

#include <shader_compiler/compiler_util.h>

#include <engine/filesystem/vfs_helper.h>

using namespace engine;
using namespace engine::render;

constexpr const size_t MAX_RENDER_OBJECT_COUNT{ 16384ull };
constexpr const size_t MAX_PRIMITIVE_COUNT{ 512ull };
constexpr const size_t MAX_MATERIAL_COUNT{ 512ull };

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
			//VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
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
std::unique_ptr<IGPUArrayHandle> allocateGPUArrayHandle(const std::string& name, size_t count, IRenderInterface* render_interface, IRenderContext* render_context)
{
	auto gpu_array = std::make_unique<CGPUArrayHandle>(render_interface, render_context);
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
}

CRenderInterface::~CRenderInterface()
{

}

void CRenderInterface::create(const FRenderInterfaceCreateInfo& rhi_ci)
{
	m_pResourceLoader->initialize(m_pVFS);

	m_pRenderContext = make_render_context(rhi_ci);
	
	m_pRenderScriptCore->create(this, m_pRenderContext.get());

	FGlobalUniformCreateInfo global_buffer_ci{};
	global_buffer_ci.uniforms[utl::fnv1a_64_hash("GlobalUniformFrameData")] = sizeof(FGlobalUniformData);
	m_pRenderContext->createGlobalDescriptor(global_buffer_ci);

	std::vector<uint8_t> render_config{};
	m_pVFS->readFile("/root/embed/rendering/test_render_config.lua", render_config);
	m_pRenderScriptCore->build("render_config", render_config);

	// Allocating gpu to cpu side buffers
	m_pRenderPacket->gpuBatches = allocateGPUArrayHandle<FGeometryGPUBatch>("GeometryBatches", MAX_RENDER_OBJECT_COUNT, this, m_pRenderContext.get());
	m_pRenderPacket->cullingInfos = allocateGPUArrayHandle<FGeometryGPUCullInfo>("CullingInfo", MAX_RENDER_OBJECT_COUNT, this, m_pRenderContext.get());
	m_pRenderPacket->sceneFrustums = allocateGPUArrayHandle<glm::vec4>("SceneFrustum", MAX_PRIMITIVE_COUNT * 6ull, this, m_pRenderContext.get());

	m_pRenderPacket->materials = allocateGPUArrayHandle<FMaterial>("MaterialStorage", MAX_MATERIAL_COUNT, this, m_pRenderContext.get());
	m_pRenderPacket->nodes = allocateGPUArrayHandle<FSceneNode>("NodeStorage", MAX_PRIMITIVE_COUNT, this, m_pRenderContext.get());
	m_pRenderPacket->skins = allocateGPUArrayHandle<glm::mat4>("SkinningJoints", MAX_PRIMITIVE_COUNT * 128ull, this, m_pRenderContext.get());


	// Directional light data
	m_pRenderPacket->dlights.lights = allocateGPUArrayHandle<FDirectionalLightCommit>("DirectionalLights", max_directional_light_count, this, m_pRenderContext.get());
	m_pRenderPacket->dlights.shadows = allocateGPUArrayHandle<FCascadedShadowCommit>("CascadedShadows", max_directional_light_shadow_count, this, m_pRenderContext.get());

	// Point light data
	m_pRenderPacket->plights.lights = allocateGPUArrayHandle<FPointLightCommit>("PointLights", max_point_light_count, this, m_pRenderContext.get());
	m_pRenderPacket->plights.shadows = allocateGPUArrayHandle<FOmniShadowCommit>("OmniShadows", max_point_light_shadow_count, this, m_pRenderContext.get());

	// Spot light data
	m_pRenderPacket->slights.lights = allocateGPUArrayHandle<FSpotLightCommit>("SpotLights", max_spot_light_count, this, m_pRenderContext.get());
	m_pRenderPacket->slights.shadows = allocateGPUArrayHandle<FSpotShadowCommit>("SpotShadows", max_spot_light_shadow_count, this, m_pRenderContext.get());

	
	m_vArrayHandles.emplace_back(m_pRenderPacket->gpuBatches.get());
	m_vArrayHandles.emplace_back(m_pRenderPacket->cullingInfos.get());
	m_vArrayHandles.emplace_back(m_pRenderPacket->sceneFrustums.get());
	m_vArrayHandles.emplace_back(m_pRenderPacket->materials.get());
	m_vArrayHandles.emplace_back(m_pRenderPacket->nodes.get());
	m_vArrayHandles.emplace_back(m_pRenderPacket->skins.get());

	m_vArrayHandles.emplace_back(m_pRenderPacket->dlights.lights.get());
	m_vArrayHandles.emplace_back(m_pRenderPacket->dlights.shadows.get());

	m_vArrayHandles.emplace_back(m_pRenderPacket->plights.lights.get());
	m_vArrayHandles.emplace_back(m_pRenderPacket->plights.shadows.get());

	m_vArrayHandles.emplace_back(m_pRenderPacket->slights.lights.get());
	m_vArrayHandles.emplace_back(m_pRenderPacket->slights.shadows.get());

	// Create draw buffers
	m_pRenderPacket->drawIndexedCommands = std::make_unique<CBufferObject>(this, m_pRenderContext.get());

	FBufferCreateInfo buffer_ci{};
	buffer_ci.flags = BUFFER_TYPE_INDIRECT_FLAG_BIT;
	buffer_ci.size = sizeof(FDrawIndexedIndirectData);
	buffer_ci.count = max_indirect_draw_count;
	m_pRenderPacket->drawIndexedCommands->create("IndexedDrawIndirectBuffer", buffer_ci);
}

void CRenderInterface::render(float dt)
{
	for (auto* array_handle : m_vArrayHandles)
		array_handle->flush();

	m_pRenderContext->beginFrameCommandBuffers();

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
	global_uniform.directionalLightCount = static_cast<uint32_t>(m_pRenderPacket->dlights.lights->size());
	global_uniform.pointLightCount = static_cast<uint32_t>(m_pRenderPacket->plights.lights->size());
	global_uniform.spotLightCount = static_cast<uint32_t>(m_pRenderPacket->slights.lights->size());
	global_uniform.rawCommandsCount = static_cast<uint32_t>(m_pRenderPacket->drawCallCount);

	FGlobalUniformUpdateInfo update_info{};
	auto& uniform = update_info.uniforms[utl::fnv1a_64_hash("GlobalUniformFrameData")];
	uniform.data = &global_uniform;
	uniform.size = sizeof(FGlobalUniformData);

	m_pRenderContext->updateGlobalDescriptorData(update_info);


	for (auto& stage : m_vRenderingSteps)
		stage->execute(dt);

	m_pRenderContext->endFrameCommandBuffers();

	if (m_pRenderContext->isSwapChainFinished())
	{
		auto to_remove = std::remove_if(m_vRenderingSteps.begin(), m_vRenderingSteps.end(),
			[](const std::unique_ptr<IRenderGraphStep>& stage) { return stage->expired(); });

		if (to_remove != m_vRenderingSteps.end())
			m_vRenderingSteps.erase(to_remove, m_vRenderingSteps.end());
	}

	m_pRenderPacket->sceneDrawCalls.clear();
	m_pRenderPacket->shadowDrawCalls.clear();

	m_pRenderPacket->gpuBatches->clear();
	m_pRenderPacket->cullingInfos->clear();
	m_pRenderPacket->sceneFrustums->clear();
	m_pRenderPacket->nodes->clear();
	m_pRenderPacket->skins->clear();

	m_pRenderPacket->dlights.lights->clear();
	m_pRenderPacket->dlights.shadows->clear();
	m_pRenderPacket->plights.lights->clear();
	m_pRenderPacket->plights.shadows->clear();
	m_pRenderPacket->slights.lights->clear();
	m_pRenderPacket->slights.shadows->clear();
}

size_t CRenderInterface::createTexture(const std::string& path)
{
	auto name = filesystem::fs::get_stem(path);
	auto ext = filesystem::fs::get_ext(path);
	auto ext_hash = utl::const_hash(ext.c_str(), ext.size());
	auto mime = static_cast<EMimeType>(ext_hash);

	size_t texture_id = m_pRenderContext->getTextureId(name);
	if (texture_id != ~0ull)
		return texture_id;

	std::vector<uint8_t> data{};
	m_pVFS->readFile(path, data);

	FTextureCreateInfo texture_ci{};
	FTextureUploadInfo texture_ui{};
	m_pResourceLoader->loadTexture(data, mime, texture_ci, texture_ui);

	return m_pRenderContext->createTexture(name, texture_ci, std::move(texture_ui));
}

size_t CRenderInterface::createShader(const std::string& name, const FGraphicsShaderCreateInfo& create_info)
{
	auto name_info = getShaderNameTag(name);
	return createShader<FGraphicsShaderCreateInfo>(name_info, create_info);
}

size_t CRenderInterface::createShader(const std::string& name, const FComputeShaderCreateInfo& create_info)
{
	auto name_info = getShaderNameTag(name);
	return createShader<FComputeShaderCreateInfo>(name_info, create_info);
}

size_t CRenderInterface::createVertexHandler(const std::string& name, const FVertexHandlerCreateInfo& handler_ci)
{
	auto vertex_handler = std::make_unique<CVertexHandler>(this, m_pRenderContext.get());
	vertex_handler->create(handler_ci);

	return m_pResources->add<IVertexHandler>(name.c_str(), std::move(vertex_handler));
}

const std::unique_ptr<IVertexHandler>& CRenderInterface::getVertexHandler(size_t id)
{
	return m_pResources->get<IVertexHandler>(id);
}

void CRenderInterface::writeDrawData(const std::vector<FGeometryRawBatch>& draw_commands)
{
	std::vector<FDrawIndexedIndirectData> draw_indirect_data{};
	std::vector<FGeometryGPUBatch> gpu_batches{};
	std::vector<FGeometryCPUBatch> cpu_batches{};
	std::vector<uint32_t> counted{};

	// Prepare gpu data
	size_t current_material{ ~0ull };
	for (const auto& command : draw_commands)
	{
		FGeometryGPUCullInfo culling_info{};
		culling_info.instanceID = command.callInfo.instanceID;

		m_pRenderPacket->cullingInfos->emplace_back(&culling_info);

		auto& vbo = getVertexHandler(command.callInfo.vboID);

		auto last_index = vbo->getLastIndex();
		auto last_vertex = vbo->getLastVertex();

		auto& indirect_command = draw_indirect_data.emplace_back();
		indirect_command.indexCount = command.range.indexCount == 0ull ? last_index : command.range.indexCount;;
		indirect_command.instanceCount = 1u;
		indirect_command.firstIndex = command.range.beginIndex;
		indirect_command.vertexOffset = command.range.beginVertex;
		indirect_command.firstInstance = 0u;
		command.range.vertexCount == 0ull ? last_vertex : command.range.vertexCount;

		indirect_command.aabb.min = command.range.bounds.min;
		indirect_command.aabb.max = command.range.bounds.max;

		// Make compact batches by material chain
		if (!gpu_batches.empty() &&
			gpu_batches.back().vboID == command.callInfo.vboID &&
			gpu_batches.back().instanceID == command.callInfo.instanceID &&
			current_material + 1 == command.callInfo.materialID)
		{
			counted.back()++;
			current_material = command.callInfo.materialID;
		}
		else
		{
			auto& new_gpu_batch = gpu_batches.emplace_back();
			new_gpu_batch = command.callInfo;
			counted.emplace_back(1ull);

			current_material = command.callInfo.materialID;
		}
	}

	// Prepare cpu data
	size_t command_offset{ 0ull };
	for (size_t idx = 0ull; idx < gpu_batches.size(); ++idx)
	{
		auto& gpu_batch = gpu_batches.at(idx);

		auto& new_cpu_batch = cpu_batches.emplace_back();
		new_cpu_batch.vboID = gpu_batch.vboID;
		new_cpu_batch.command.bufferId = m_pRenderPacket->drawIndexedCommands->handle();
		new_cpu_batch.command.offset = command_offset * sizeof(FDrawIndexedIndirectData);
		new_cpu_batch.command.count = counted.at(idx);
		new_cpu_batch.command.stride = sizeof(FDrawIndexedIndirectData);

		command_offset += new_cpu_batch.command.count;
	}

	m_pRenderPacket->sceneDrawCalls = std::move(cpu_batches);

	// Copy indexed draw comands
	m_pRenderPacket->drawIndexedCommands->map();
	m_pRenderPacket->drawIndexedCommands->write(draw_indirect_data.data(), draw_indirect_data.size());
	m_pRenderPacket->drawIndexedCommands->unmap();

	for (const auto& gpu_batch : gpu_batches)
		m_pRenderPacket->gpuBatches->emplace_back(&gpu_batch);

	m_pRenderPacket->drawCallCount = command_offset;

	m_pRenderPacket->camera.frustum.update(m_pRenderPacket->camera.view, m_pRenderPacket->camera.projection);
	for (const auto& side : m_pRenderPacket->camera.frustum.frustumSides)
		m_pRenderPacket->sceneFrustums->emplace_back(&side);
}

void CRenderInterface::updateSurfaceSize(int32_t width, int32_t height)
{
	m_pRenderContext->updateSurface(width, height);
}

const std::unique_ptr<FRenderDataPacket>& CRenderInterface::getRenderPacket()
{
	return m_pRenderPacket;
}

IRenderContext* CRenderInterface::getRenderContext()
{
	return m_pRenderContext.get();
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

void CRenderInterface::createRendering(const FRenderingStepCreateInfo& rendering_ci)
{
	auto& new_step = m_vRenderingSteps.emplace_back();

	if (rendering_ci.rendering.materialPass)
	{
		m_matPassRTs = rendering_ci.rendering.renderTargets;
		m_matPassDepth = rendering_ci.rendering.depthTarget;
		m_matPassStencil = rendering_ci.rendering.stencilTarget;
	}

	if (rendering_ci.type == RENDERING_STEP_SYNCHRONIZATION_TYPE)
	{
		if (rendering_ci.synchronization.type == SYNCHRONIZATION_SEMAPHORE_TYPE)
			new_step = std::make_unique<CSemaphoreRenderingStep>(this, m_pRenderContext.get());
		else if (rendering_ci.synchronization.type == SYNCHRONIZATION_BARRIER_TYPE)
			new_step = std::make_unique<CBarrierRenderingStep>(this, m_pRenderContext.get());
	}
	else if (rendering_ci.type == RENDERING_STEP_RENDERING_TYPE)
	{
		if(rendering_ci.rendering.type == RENDERING_FULLSCREEN_TYPE)
			new_step = std::make_unique<CFullscreenRenderingStep>(this, m_pRenderContext.get());
		else if (rendering_ci.rendering.type == RENDERING_GEOMETRY_TYPE)
			new_step = std::make_unique<CGeometryRenderingStep>(this, m_pRenderContext.get());
		else if (rendering_ci.rendering.type == RENDERING_CSM_TYPE)
			new_step = std::make_unique<CCascadedShadowMapsRenderingStep>(this, m_pRenderContext.get());
		else if(rendering_ci.rendering.type == RENDERING_RENDER_SCRIPT_TYPE)
			new_step = std::make_unique<CRenderScriptRenderingStep>(this, m_pRenderContext.get());
	}
	else if (rendering_ci.type == RENDERING_STEP_COMPUTE_TYPE)
	{
		if (rendering_ci.compute.type == COMPUTE_DEFAULT_TYPE)
			new_step = std::make_unique<CComputeRenderingStep>(this, m_pRenderContext.get());
		else if (rendering_ci.compute.type == COMPUTE_GEOMETRY_TYPE)
			new_step = std::make_unique<CComputeGeometryRenderingStep>(this, m_pRenderContext.get());
		else if (rendering_ci.compute.type == COMPUTE_CUBEMAP_TYPE)
			new_step = std::make_unique<CComputeCubemapRenderingStep>(this, m_pRenderContext.get());
		else if (rendering_ci.compute.type == COMPUTE_CULLING_TYPE)
			new_step = std::make_unique<CComputeCullingRenderingStep>(this, m_pRenderContext.get());
	}

	new_step->create(rendering_ci);

	//auto& stage = m_vRenderStages.emplace_back();
	//stage = std::make_unique<CDynamicRenderStage>();
	//
	//stage->m_renderingInfo.flipViewport = rendering_ci.flipViewport;
	//stage->m_renderingInfo.queue = rendering_ci.queue;
	//stage->m_renderingInfo.colorAttachments = rendering_ci.renderTargets;
	//stage->m_renderingInfo.depthAttachment = rendering_ci.depthTarget;
	//stage->m_renderingInfo.stencilAttachment = rendering_ci.stencilTarget;
	//
	//// Load render script
	//std::vector<uint8_t> render_script{};
	//m_pVFS->readFile(rendering_ci.renderScript, render_script);
	//stage->m_pScriptExecutor = m_pRenderScriptCore->build(filesystem::fs::get_stem(rendering_ci.renderScript), render_script);
	//stage->m_pScriptExecutor->addToEnvironment("this", stage.get());
	//
	//// At first i should create shader 
	//// After that shader object
	//stage->create(rendering_ci.shaderName);
	//
	//// Apply shader inputh
	//for (auto& [binding_point, texture] : rendering_ci.shaderInputs)
	//	stage->m_pStageShader->bindTexture(binding_point.c_str(), texture);
}