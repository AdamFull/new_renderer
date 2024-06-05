#include "RenderInterface.h"

#include "render_api/vulkan/VkRenderContext.h"

#include "RenderWorld.h"
#include "render_resouces/ResourceManager.h"
#include "render_graph/RenderGraph.h"

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
			VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME,
			VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
			VK_KHR_SPIRV_1_4_EXTENSION_NAME,
			VK_EXT_MESH_SHADER_EXTENSION_NAME,
			VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME,
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

		render_context->Create(context_ci);
	}

	return render_context;
}

CRenderInterface::CRenderInterface(filesystem::IVirtualFileSystemInterface* vfs)
{
	m_pVFS = vfs;

	m_pResourceManager = std::make_unique<CRenderResourceManager>(this, vfs);
	m_pRenderPacket = std::make_unique<FRenderDataPacket>();

	m_pRenderGraph = std::make_unique<CRenderGraph>(this);
	m_pRenderWorld = std::make_unique<CRenderWorld>(this);
}

CRenderInterface::~CRenderInterface()
{

}

void CRenderInterface::Create(const FRenderInterfaceCreateInfo& rhi_ci)
{
	m_pRenderContext = make_render_context(rhi_ci);

	FGlobalUniformCreateInfo global_buffer_ci{};
	global_buffer_ci.uniforms[shader_bindings::global_uniform_frame_data] = sizeof(FGlobalUniformData);
	global_buffer_ci.uniforms[shader_bindings::global_buffer_references] = sizeof(FGlobalBufferReference);
	global_buffer_ci.uniforms[shader_bindings::global_post_process_data] = sizeof(FGlobalPostProcessingData);
	m_pRenderContext->CreateGlobalDescriptor(global_buffer_ci);

	m_pResourceManager->Create();
	m_pRenderWorld->Create();

	utl::vector<uint8_t> render_config{};
	m_pVFS->ReadFile("/root/embed/rendering/render_config.xml", render_config);
	utl::vector<uint8_t> render_config_schema{};
	m_pVFS->ReadFile("/root/embed/rendering/render_config.xsd", render_config_schema);
	m_pRenderGraph->Create(render_config, render_config_schema);

	m_pRenderGraph->Build();
}

void CRenderInterface::PostLoad()
{
	m_pRenderGraph->PostLoad();
}

void CRenderInterface::Render(f32 dt)
{
	m_pRenderGraph->Update(dt);
	m_pRenderWorld->Begin(dt);

	m_pRenderContext->BeginFrame();

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
	global_references.nodeBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eNodeBuffer)->Buffer()->Address();
	global_references.matrixBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eMatricesBuffer)->Buffer()->Address();
	global_references.materialBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eMaterialBuffer)->Buffer()->Address();
	global_references.meshBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eMeshBuffer)->Buffer()->Address();

	global_references.lightsBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eLightBuffer)->Buffer()->Address();

	global_references.cascadedShadowBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eCascadedShadowBuffer)->Buffer()->Address();
	global_references.omniShadowBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eOmniShadowBuffer)->Buffer()->Address();
	global_references.spotShadowBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eSpotShadowBuffer)->Buffer()->Address();

	global_references.drawInfoBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eDrawMeshInfoBuffer)->Buffer()->Address();
	global_references.rawDrawInfoBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eRawDrawMeshInfoBuffer)->Buffer()->Address();
	global_references.clusterBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eClusterInfoBuffer)->Buffer()->Address();
	global_references.frustumSidesBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eFrustumBuffer)->Buffer()->Address();

	global_references.indirectCommandBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eDrawCommandBuffer)->Buffer()->Address();
	global_references.indirectCountBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eCountBuffer)->Buffer()->Address();
	global_references.indirectTaskCommandBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eMeshCommandBuffer)->Buffer()->Address();

	global_references.textureRemapBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eTextureRemapBuffer)->Buffer()->Address();
	global_references.cameraBuffer = m_pRenderWorld->GetDrawBuffer(ERenderBufferIndices::eCameraBuffer)->Buffer()->Address();

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
	m_pRenderGraph->Execute(dt);

	m_pRenderContext->EndFrame();

	m_pRenderWorld->End();

	++m_frameCounter;
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

const std::unique_ptr<IRenderContext>& CRenderInterface::GetRenderContext()
{
	return m_pRenderContext;
}

const std::unique_ptr<IRenderResourceManager>& CRenderInterface::GetResourceManager()
{
	return m_pResourceManager;
}