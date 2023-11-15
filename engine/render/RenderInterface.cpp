#include <RenderInterface.h>

#include <render_api/vulkan/VkRenderContext.h>
#include <rendering_steps/FullscreenRenderingStep.h>
#include <rendering_steps/RenderScriptRenderingStep.h>
#include <rendering_steps/SynchronizationRenderingStep.h>
#include <Material.h>

#include <shader_compiler/compiler_util.h>

#include <filesystem/vfs_helper.h>

using namespace engine;
using namespace engine::render;

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
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
			VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
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
}

CRenderInterface::~CRenderInterface()
{

}

void CRenderInterface::create(const FRenderInterfaceCreateInfo& rhi_ci)
{
	m_pResourceLoader = std::make_unique<CResourceLoader>();
	m_pResourceLoader->initialize(m_pVFS);

	m_pRenderContext = make_render_context(rhi_ci);

	m_pRenderScriptCore = std::make_unique<CRenderScriptCore>();
	m_pRenderScriptCore->create(this, m_pRenderContext.get());

	std::vector<uint8_t> render_config{};
	m_pVFS->readFile("/root/embed/rendering/test_render_config.lua", render_config);
	m_pRenderScriptCore->build("render_config", render_config);

	m_pResources = std::make_unique<CResourceContainer>();
	m_pResources->registerResource<CMaterial>();
}

void CRenderInterface::render(float dt)
{
	m_pRenderContext->beginFrameCommandBuffers();

	for (auto& stage : m_vRenderingSteps)
		stage->execute(dt);

	m_pRenderContext->endFrameCommandBuffers();
}

size_t CRenderInterface::createTexture(const std::string& path)
{
	auto name = filesystem::fs::get_stem(path);
	auto ext = filesystem::fs::get_ext(path);
	auto ext_hash = utl::const_hash(ext.c_str(), ext.size());
	auto mime = static_cast<EMimeType>(ext_hash);

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

size_t CRenderInterface::createMaterial(const std::string& name, const FMaterialCreateInfo& material_ci)
{
	auto material = std::make_unique<CMaterial>();

	std::string shader_name = material_ci.baseShaderName;
	for (const auto& define : material_ci.definitions)
		shader_name += ":" + define;

	// TODO: i don't know how to set output targets for this shader. 
	FGraphicsShaderCreateInfo shader_ci{};
	shader_ci.cullMode = SHADER_CULL_MODE_BACK_BIT;
	shader_ci.frontFace = SHADER_FRONT_FACE_CCW;
	shader_ci.primitiveTopology = SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	shader_ci.colorAttachments = m_matPassRTs;
	shader_ci.depthAttachment = m_matPassDepth;
	shader_ci.stencilAttachment = m_matPassStencil;

	auto shader_id = createShader(shader_name, shader_ci);
	auto shader_reflection = m_pRenderContext->getShaderReflection(shader_id);

	material->m_pShader->create(shader_id, shader_reflection);

	// Bind default textures to shader
	for (auto& [binding_point, texture] : material_ci.textures)
		material->m_pShader->bindTexture(binding_point.c_str(), texture);

	return m_pResources->add<CMaterial>(name.c_str(), std::move(material));
}

void CRenderInterface::updateSurfaceSize(int32_t width, int32_t height)
{
	m_pRenderContext->updateSurface(width, height);
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