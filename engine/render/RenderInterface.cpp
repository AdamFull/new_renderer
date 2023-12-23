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

#include "VertexHandler.h"
#include "BufferObject.h"
#include "GPUArrayHandle.h"

#include <shader_compiler/compiler_util.h>

#include <engine/filesystem/vfs_helper.h>

using namespace engine;
using namespace engine::render;

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

CRenderInterface::CRenderInterface(filesystem::IVirtualFileSystemInterface* vfs)
{
	m_pVFS = vfs;

	m_pResourceLoader = std::make_unique<CResourceLoader>();
	m_pRenderScriptCore = std::make_unique<CRenderScriptCore>();
	m_pRenderPacket = std::make_unique<FRenderDataPacket>();

	m_pResources = std::make_unique<CResourceContainer>();
	m_pResources->registerResource<IVertexHandler>();

	m_pGlobalUniformHandler = std::make_unique<CUniformHandle>();
}

CRenderInterface::~CRenderInterface()
{

}

void CRenderInterface::create(const FRenderInterfaceCreateInfo& rhi_ci)
{
	m_pResourceLoader->initialize(m_pVFS);

	m_pRenderContext = make_render_context(rhi_ci);
	
	m_pRenderScriptCore->create(this, m_pRenderContext.get());

	// TODO: make it better
	std::vector<std::pair<size_t, size_t>> sizemap{};
	sizemap.emplace_back(utl::fnv1a_64_hash("view"), 64ull);					// mat4 view;
	sizemap.emplace_back(utl::fnv1a_64_hash("invView"), 64ull);				// mat4 invView;
	sizemap.emplace_back(utl::fnv1a_64_hash("projection"), 64ull);			// mat4 projection;
	sizemap.emplace_back(utl::fnv1a_64_hash("invProjection"), 64ull);		// mat4 invProjection;
	sizemap.emplace_back(utl::fnv1a_64_hash("viewProjection"), 64ull);		// mat4 viewProjection;
	sizemap.emplace_back(utl::fnv1a_64_hash("invViewProjection"), 64ull);	// mat4 invViewProjection;
	sizemap.emplace_back(utl::fnv1a_64_hash("reprojection"), 64ull);		// mat4 reprojection;
	sizemap.emplace_back(utl::fnv1a_64_hash("viewportDimension"), 8ull);	// vec2 viewportDimension;
	sizemap.emplace_back(utl::fnv1a_64_hash("invViewportDimension"), 8ull);	// vec2 viewportDimension;
	sizemap.emplace_back(utl::fnv1a_64_hash("nearPlane"), 4ull);			// float nearPlane;
	sizemap.emplace_back(utl::fnv1a_64_hash("farPlane"), 4ull);				// float farPlane;
	sizemap.emplace_back(utl::fnv1a_64_hash("directionalLightCount"), 4ull);				// uint directionalLightCount;
	sizemap.emplace_back(utl::fnv1a_64_hash("pointLightCount"), 4ull);				// uint pointLightCount;
	sizemap.emplace_back(utl::fnv1a_64_hash("spotLightCount"), 4ull);				// uint spotLightCount;
	sizemap.emplace_back(utl::fnv1a_64_hash("frustumPlanes"), 96ull);		// vec4 frustumPlanes[6];

	FGlobalUniformCreateInfo global_buffer_ci{};
	auto& ublock = global_buffer_ci.uniforms[utl::fnv1a_64_hash("GlobalUniformFrameData")];
	for (const auto& [name, size] : sizemap)
	{
		ublock.uniforms.emplace(name, FReflectionUniform{ 0u, 0u, ublock.size, size });
		ublock.size += size;
	}

	m_pGlobalUniformHandler->create(ublock);
	m_pRenderContext->createGlobalDescriptor(global_buffer_ci);

	std::vector<uint8_t> render_config{};
	m_pVFS->readFile("/root/embed/rendering/test_render_config.lua", render_config);
	m_pRenderScriptCore->build("render_config", render_config);


	m_pRenderPacket->materials = std::make_unique<CGPUArrayHandle>(this, m_pRenderContext.get());
	m_pRenderPacket->materials->create("MaterialStorage", sizeof(FMaterial), MAX_MATERIAL_COUNT);

	m_pRenderPacket->nodes = std::make_unique<CGPUArrayHandle>(this, m_pRenderContext.get());
	m_pRenderPacket->nodes->create("NodeStorage", sizeof(FSceneNode), MAX_PRIMITIVE_COUNT);

	m_pRenderPacket->skins = std::make_unique<CGPUArrayHandle>(this, m_pRenderContext.get());
	m_pRenderPacket->skins->create("SkinningJoints", sizeof(glm::mat4) * 128ull, MAX_PRIMITIVE_COUNT);


	// Directional light data
	m_pRenderPacket->dlights.lights = std::make_unique<CGPUArrayHandle>(this, m_pRenderContext.get());
	m_pRenderPacket->dlights.lights->create("DirectionalLights", sizeof(FDirectionalLightCommit), max_directional_light_count);

	m_pRenderPacket->dlights.shadows = std::make_unique<CGPUArrayHandle>(this, m_pRenderContext.get());
	m_pRenderPacket->dlights.shadows->create("CascadedShadows", sizeof(FCascadedShadowCommit), max_directional_light_shadow_count);

	// Point light data
	m_pRenderPacket->plights.lights = std::make_unique<CGPUArrayHandle>(this, m_pRenderContext.get());
	m_pRenderPacket->plights.lights->create("PointLights", sizeof(FPointLightCommit), max_point_light_count);

	m_pRenderPacket->plights.shadows = std::make_unique<CGPUArrayHandle>(this, m_pRenderContext.get());
	m_pRenderPacket->plights.shadows->create("OmniShadows", sizeof(FOmniShadowCommit), max_point_light_shadow_count);

	// Spot light data
	m_pRenderPacket->slights.lights = std::make_unique<CGPUArrayHandle>(this, m_pRenderContext.get());
	m_pRenderPacket->slights.lights->create("SpotLights", sizeof(FSpotLightCommit), max_spot_light_count);

	m_pRenderPacket->slights.shadows = std::make_unique<CGPUArrayHandle>(this, m_pRenderContext.get());
	m_pRenderPacket->slights.shadows->create("SpotShadows", sizeof(FSpotShadowCommit), max_spot_light_shadow_count);

	// Create draw buffers
	m_pRenderPacket->drawCommands = std::make_unique<CBufferObject>(this, m_pRenderContext.get());
	m_pRenderPacket->drawIndexedCommands = std::make_unique<CBufferObject>(this, m_pRenderContext.get());

	FBufferCreateInfo buffer_ci{};
	buffer_ci.flags = BUFFER_TYPE_INDIRECT_FLAG_BIT;
	buffer_ci.size = sizeof(FDrawIndirectData);
	buffer_ci.count = max_indirect_draw_count;
	m_pRenderPacket->drawCommands->create("DrawIndirectBuffer", buffer_ci);

	buffer_ci.size = sizeof(FDrawIndexedIndirectData);
	buffer_ci.count = max_indirect_draw_count;
	m_pRenderPacket->drawIndexedCommands->create("IndexedDrawIndirectBuffer", buffer_ci);
}

void CRenderInterface::render(float dt)
{

	//m_pRenderPacket->resources->flush();
	m_pRenderPacket->materials->flush();
	m_pRenderPacket->nodes->flush();
	m_pRenderPacket->skins->flush();

	m_pRenderPacket->dlights.lights->flush(m_pRenderPacket->dlights.lcount);
	m_pRenderPacket->dlights.shadows->flush(m_pRenderPacket->dlights.scount);

	m_pRenderPacket->plights.lights->flush(m_pRenderPacket->plights.lcount);
	m_pRenderPacket->plights.shadows->flush(m_pRenderPacket->plights.scount);

	m_pRenderPacket->slights.lights->flush(m_pRenderPacket->slights.lcount);
	m_pRenderPacket->slights.shadows->flush(m_pRenderPacket->slights.scount);


	m_pRenderContext->beginFrameCommandBuffers();

	// TODO: make it better
	m_pRenderPacket->camera.frustum.update(m_pRenderPacket->camera.view, m_pRenderPacket->camera.projection);
	m_pGlobalUniformHandler->set("view", m_pRenderPacket->camera.view);
	m_pGlobalUniformHandler->set("invView", m_pRenderPacket->camera.invView);
	m_pGlobalUniformHandler->set("projection", m_pRenderPacket->camera.projection);
	m_pGlobalUniformHandler->set("invProjection", m_pRenderPacket->camera.invProjection);
	m_pGlobalUniformHandler->set("viewProjection", m_pRenderPacket->camera.viewProjection);
	m_pGlobalUniformHandler->set("invViewProjection", m_pRenderPacket->camera.invViewProjection);
	//m_pGlobalUniformHandler->set("reprojection", m_pRenderPacket->camera.reprojection);
	m_pGlobalUniformHandler->set("viewportDimension", m_pRenderPacket->camera.dimensions);
	m_pGlobalUniformHandler->set("invViewportDimension", 1.f / m_pRenderPacket->camera.dimensions);
	m_pGlobalUniformHandler->set("nearPlane", m_pRenderPacket->camera.nearp);
	m_pGlobalUniformHandler->set("farPlane", m_pRenderPacket->camera.farp);
	m_pGlobalUniformHandler->set("directionalLightCount", m_pRenderPacket->dlights.lcount);
	m_pGlobalUniformHandler->set("pointLightCount", m_pRenderPacket->plights.lcount);
	m_pGlobalUniformHandler->set("spotLightCount", m_pRenderPacket->slights.lcount);
	m_pGlobalUniformHandler->set("frustumPlanes", m_pRenderPacket->camera.frustum.frustumSides);

	FGlobalUniformUpdateInfo update_info{};
	auto& uniform = update_info.uniforms[utl::fnv1a_64_hash("GlobalUniformFrameData")];
	auto& block_data = m_pGlobalUniformHandler->data();
	uniform.data = block_data.data();
	uniform.size = block_data.size();

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

	m_pRenderPacket->rawCommands->clear();
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

void CRenderInterface::drawPrimitive(const FGeometryDrawCallCommand& raw_comand)
{
	auto& raw_commands = m_pRenderPacket->rawCommands;
	size_t index{ 0ull };

	raw_commands->insert(&raw_comand, index);

	auto* command = reinterpret_cast<FGeometryDrawCallCommand*>(raw_commands->at(index));

	auto& vbo = getVertexHandler(raw_comand.vboID);

	auto last_index = vbo->getLastIndex();
	command->range.indexCount = command->range.indexCount == 0ull ? last_index : command->range.indexCount;
	
	auto last_vertex = vbo->getLastVertex();
	command->range.vertexCount = command->range.vertexCount == 0ull ? last_vertex : command->range.vertexCount;
}

void CRenderInterface::fillDrawBuffer(const std::vector<FGeometryBatch>& batches, const std::unique_ptr<IBufferObject>& buffer, const std::unique_ptr<IBufferObject>& indexed_buffer, std::vector<FGeometryDrawCall>& draw_calls, size_t& offset, size_t& indexed_offset)
{
	for (const auto& batch : batches)
	{
		auto& vbo = getVertexHandler(batch.vboId);
		if (vbo)
		{
			auto last_index = vbo->getLastIndex();
			auto last_vertex = vbo->getLastVertex();

			auto& draw_call = draw_calls.emplace_back();
			draw_call.vboId = batch.vboId;
			draw_call.materialId = batch.materialId;
			draw_call.node = batch.node;

			if (last_index == 0ull)
			{
				std::vector<FDrawIndirectData> draw_commands{};

				draw_call.command.bufferId = indexed_buffer->handle();
				draw_call.command.offset = offset;
				draw_call.command.count = batch.ranges.size();

				for (auto& range : batch.ranges)
				{
					auto& command = draw_commands.emplace_back();
					command.vertexCount = range.vertexCount == 0ull ? last_vertex : range.vertexCount;
					command.instanceCount = 1;
					command.firstVertex = range.beginVertex;
					command.firstInstance = 0;
				}

				if (!draw_commands.empty())
				{
					buffer->map();
					auto* buffer_ptr = buffer->at(0ull);
					std::memcpy(buffer_ptr + draw_call.command.offset, &draw_commands[0], draw_commands.size() * sizeof(FDrawIndirectData));
					buffer->unmap();
				}

				indexed_offset += draw_commands.size() * sizeof(FDrawIndirectData);
			}
			else
			{
				std::vector<FDrawIndexedIndirectData> draw_indexed_commands{};

				draw_call.command.bufferId = indexed_buffer->handle();
				draw_call.command.offset = indexed_offset;
				draw_call.command.count = batch.ranges.size();

				for (auto& range : batch.ranges)
				{
					auto& command = draw_indexed_commands.emplace_back();
					command.indexCount = range.indexCount == 0ull ? last_index : range.indexCount;
					command.instanceCount = 1;
					command.firstIndex = range.beginIndex;
					command.vertexOffset = 0;
					command.firstInstance = 0;
				}

				if (!draw_indexed_commands.empty())
				{
					indexed_buffer->map();
					auto* buffer_ptr = indexed_buffer->at(0ull);
					std::memcpy(buffer_ptr + draw_call.command.offset, &draw_indexed_commands[0], draw_indexed_commands.size() * sizeof(FDrawIndexedIndirectData));
					indexed_buffer->unmap();
				}

				indexed_offset += draw_indexed_commands.size() * sizeof(FDrawIndexedIndirectData);
			}
		}
	}
}

void CRenderInterface::fillDrawBuffers(const std::vector<FGeometryDrawData>& draw_data)
{
	size_t offset{ 0ull }, indexed_offset{ 0ull };
	std::vector<FGeometryBatch> scene_batches{};
	createGeometryBatches(draw_data, scene_batches);
	fillDrawBuffer(scene_batches, m_pRenderPacket->drawCommands, m_pRenderPacket->drawIndexedCommands, m_pRenderPacket->sceneDrawCalls, offset, indexed_offset);

	std::vector<FGeometryBatch> shadow_batches{};
	createGeometryBatches(draw_data, shadow_batches, GEOMETRY_BATCHER_MERGE_DRAWCALLS_FLAG_BIT);
	fillDrawBuffer(shadow_batches, m_pRenderPacket->drawCommands, m_pRenderPacket->drawIndexedCommands, m_pRenderPacket->shadowDrawCalls, offset, indexed_offset);
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
		else if (rendering_ci.rendering.type == RENDERING_COMPUTE_TYPE)
			new_step = std::make_unique<CComputeRenderingStep>(this, m_pRenderContext.get());
		else if (rendering_ci.rendering.type == RENDERING_COMPUTE_GEOMETRY_TYPE)
			new_step = std::make_unique<CComputeGeometryRenderingStep>(this, m_pRenderContext.get());
		else if (rendering_ci.rendering.type == RENDERING_COMPUTE_CUBEMAP_TYPE)
			new_step = std::make_unique<CComputeCubemapRenderingStep>(this, m_pRenderContext.get());
		else if (rendering_ci.rendering.type == RENDERING_CSM_TYPE)
			new_step = std::make_unique<CCascadedShadowMapsRenderingStep>(this, m_pRenderContext.get());
		else if(rendering_ci.rendering.type == RENDERING_RENDER_SCRIPT_TYPE)
			new_step = std::make_unique<CRenderScriptRenderingStep>(this, m_pRenderContext.get());
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