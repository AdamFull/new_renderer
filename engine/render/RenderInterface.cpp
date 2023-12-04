#include "RenderInterface.h"

#include "render_api/vulkan/VkRenderContext.h"
#include "rendering_steps/FullscreenRenderingStep.h"
#include "rendering_steps/RenderScriptRenderingStep.h"
#include "rendering_steps/SynchronizationRenderingStep.h"
#include "rendering_steps/VisibilityBufferRenderingStep.h"
#include "rendering_steps/ComputeRenderingStep.h"

#include "VertexHandler.h"
#include "BufferObject.h"

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
	sizemap.emplace_back(utl::fnv1a_64_hash("viewDirection"), 16ull);		// vec3 viewDirection;
	sizemap.emplace_back(utl::fnv1a_64_hash("viewportDimension"), 8ull);		// vec2 viewportDimension;
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

	// Create material storage buffer
	{
		FBufferCreateInfo buffer_ci{};
		buffer_ci.flags = BUFFER_TYPE_STORAGE_FLAG_BIT;
		buffer_ci.size = sizeof(FMaterial);
		buffer_ci.count = MAX_MATERIAL_COUNT;

		auto& buffer = m_pRenderPacket->materialBuffer;
		buffer = std::make_unique<CBufferObject>(this, m_pRenderContext.get());
		buffer->create("global_material_buffer", buffer_ci);
	}

	// Create node storage buffer
	{
		FBufferCreateInfo buffer_ci{};
		buffer_ci.flags = BUFFER_TYPE_STORAGE_FLAG_BIT;
		buffer_ci.size = sizeof(glm::mat4);
		buffer_ci.count = MAX_PRIMITIVE_COUNT;

		auto& buffer = m_pRenderPacket->nodeBuffer;
		buffer = std::make_unique<CBufferObject>(this, m_pRenderContext.get());
		buffer->create("global_primitive_data_buffer", buffer_ci);
	}

	// Create skin storage buffer
	{
		FBufferCreateInfo buffer_ci{};
		buffer_ci.flags = BUFFER_TYPE_STORAGE_FLAG_BIT;
		buffer_ci.size = sizeof(glm::mat4);
		buffer_ci.count = MAX_PRIMITIVE_COUNT * 128ull;

		auto& buffer = m_pRenderPacket->skinBuffer;
		buffer = std::make_unique<CBufferObject>(this, m_pRenderContext.get());
		buffer->create("global_joints_buffer", buffer_ci);
	}
}

void CRenderInterface::render(float dt)
{
	// Update materials
	{
		auto& buffer = m_pRenderPacket->materialBuffer;
	
		buffer->map();
		for (size_t idx = 0ull; idx < m_pRenderPacket->materials.size(); ++idx)
		{
			auto& material = m_pRenderPacket->materials.at(idx);
			auto* dst = buffer->at(idx);
			std::memcpy(dst, &material.parameters, sizeof(FMaterial));
		}
		buffer->unmap();
	}

	// Update nodes
	{
		auto& buffer = m_pRenderPacket->nodeBuffer;

		buffer->map();
		for (auto& [node, model] : m_pRenderPacket->nodes)
		{
			auto* dst = buffer->at(node);
			std::memcpy(dst, &model, sizeof(glm::mat4));
		}
		buffer->unmap();
	}

	// Update skins
	{
		auto& buffer = m_pRenderPacket->skinBuffer;

		size_t offset{ 0ull };
		buffer->map();
		for (auto& joints : m_pRenderPacket->skins)
		{
			auto* dst = buffer->at(offset);
			std::memcpy(dst, joints.data(), sizeof(glm::mat4) * joints.size());
			offset += joints.size();
		}
		buffer->unmap();
	}

	m_pRenderContext->beginFrameCommandBuffers();

	// TODO: make it better
	m_pRenderPacket->camera.frustum.update(m_pRenderPacket->camera.view, m_pRenderPacket->camera.projection);
	m_pGlobalUniformHandler->set("view", m_pRenderPacket->camera.view);
	m_pGlobalUniformHandler->set("invView", m_pRenderPacket->camera.invView);
	m_pGlobalUniformHandler->set("projection", m_pRenderPacket->camera.projection);
	m_pGlobalUniformHandler->set("invProjection", m_pRenderPacket->camera.invProjection);
	m_pGlobalUniformHandler->set("viewProjection", m_pRenderPacket->camera.viewProjection);
	m_pGlobalUniformHandler->set("invViewProjection", m_pRenderPacket->camera.invViewProjection);
	m_pGlobalUniformHandler->set("viewDirection", m_pRenderPacket->camera.viewPos);
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

	m_pRenderPacket->sceneBatches.clear();

	for (auto& batches : m_pRenderPacket->directionalLights.shadows.batches)
	{
		for (auto& batch : batches)
			batch.clear();
	}

	for (auto& batches : m_pRenderPacket->pointLights.shadows.batches)
	{
		for (auto& batch : batches)
			batch.clear();
	}

	for (auto& batch : m_pRenderPacket->spotLights.shadows.batches)
		batch.clear();

	m_pRenderPacket->skins.clear();
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
		new_step = std::make_unique<CSynchronizationRenderingStep>(this, m_pRenderContext.get());
	else if (rendering_ci.type == RENDERING_STEP_RENDERING_TYPE)
	{
		if(rendering_ci.rendering.type == RENDERING_FULLSCREEN_TYPE)
			new_step = std::make_unique<CFullscreenRenderingStep>(this, m_pRenderContext.get());
		else if (rendering_ci.rendering.type == RENDERING_GEOMETRY_TYPE)
			new_step = std::make_unique<CVisibilityBufferRenderingStep>(this, m_pRenderContext.get());
		else if (rendering_ci.rendering.type == RENDERING_COMPUTE_TYPE)
			new_step = std::make_unique<CComputeRenderingStep>(this, m_pRenderContext.get());
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