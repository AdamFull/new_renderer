#include "RenderGraph.h"

#include "render_passes/CascadedShadowMapPass.h"
#include "render_passes/ComputeCubemapPass.h"
#include "render_passes/ComputeCullingPass.h"
#include "render_passes/ComputeGeometryPass.h"
#include "render_passes/ComputePass.h"
#include "render_passes/FullscreenPass.h"
#include "render_passes/GeometryPass.h"

#include <logger/logger.h>
#include <glm/gtc/matrix_transform.hpp>

using namespace engine::render;

struct FResourceAccessInfo
{
	ERenderResourceDependencyBarrierType type{};
	u64 resource{ ~0ull };
	u64 srcPassIndex{ ~0ull };
	u64 dstPassIndex{ ~0ull };
};

void filter_pass_queue(stl::vector<u64>& pass_list)
{
	stl::unordered_set<u64> seen;

	pass_list.erase(stl::remove_if(pass_list.begin(), pass_list.end(),
		[&seen](const auto& item) { return !seen.insert(item).second; }),
		pass_list.end());
}

CRenderGraph::CRenderGraph(IRenderInterface* render_interface)
{
	m_pRenderInterface = render_interface;
}

void CRenderGraph::create()
{
	auto& render_context = m_pRenderInterface->getRenderContext();
	auto surface_resolution = render_context->getRenderResolution();

	{
		addStaticResource("CasualDay4K", "/root/CasualDay4K.ktx2");
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = render_context->getApiFormat(EApiDataFormat::eRG16_SFLOAT);
		texture_ci.typeFlags = ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage;
		texture_ci.setFixedSize2D(1024);
		addRenderResource("brdf_lut_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = render_context->getApiFormat(EApiDataFormat::eRGBA32_SFLOAT);
		texture_ci.typeFlags = ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eCubemap | ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage;
		texture_ci.setFixedSize2D(64);
		addRenderResource("irradiance_cube_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = render_context->getApiFormat(EApiDataFormat::eRGBA16_SFLOAT);
		texture_ci.typeFlags = ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eCubemap | ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage;
		texture_ci.setFixedSize2D(512);
		texture_ci.enableMipMap();
		addRenderResource("prefiltered_cube_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = render_context->getDepthFormat(false);
		texture_ci.typeFlags = ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eArray | ETextureCreateFlagBits::eSamplable;
		texture_ci.setFixedSize2D(cascade_shadow_map_resolution);
		texture_ci.arrayLayers = cascade_shadow_map_cascade_count * max_directional_light_shadow_count;
		addRenderResource("cascaded_shadows_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = render_context->getApiFormat(EApiDataFormat::eRGBA8_UNORM);
		texture_ci.typeFlags = ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eSamplable;
		texture_ci.setSizeScale(1.f);
		addRenderResource("visbuffer_geometry_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = render_context->getApiFormat(EApiDataFormat::eRGBA8_UNORM);
		texture_ci.typeFlags = ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eSamplable;
		texture_ci.setSizeScale(1.f);
		addRenderResource("visbuffer_matobj_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = render_context->getApiFormat(EApiDataFormat::eRGBA8_UNORM);
		texture_ci.typeFlags = ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eSamplable;
		texture_ci.setSizeScale(1.f);
		addRenderResource("visbuffer_skinvbo_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = render_context->getDepthFormat(false);
		texture_ci.typeFlags = ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eSamplable;
		texture_ci.setSizeScale(1.f);
		addRenderResource("zbuffer_depth_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = render_context->getApiFormat(EApiDataFormat::eRGBA16_SFLOAT);
		texture_ci.typeFlags = ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage;
		texture_ci.setSizeScale(1.f);
		addRenderResource("final_lighting_target", texture_ci);
	}

	{
		setBackbufferResourceName("present_target");
	}








	{
		FRenderConfigComputeCreateInfo pass_ci{};
		pass_ci.oneTime = true;
		pass_ci.type = EComputeType::eDefault;
		pass_ci.queue = EQueueTypeFlagBits::eGraphics;
		pass_ci.shader = "lighting:brgflut_generator";

		pass_ci.sizeTarget = "brdf_lut_target";
		pass_ci.bindWriteResource("out_color", "brdf_lut_target");

		addRenderPass("generate_brdflut", pass_ci);
	}

	{
		FRenderConfigComputeCreateInfo pass_ci{};
		pass_ci.oneTime = true;
		pass_ci.type = EComputeType::eDefault;
		pass_ci.queue = EQueueTypeFlagBits::eGraphics;
		pass_ci.shader = "lighting:irradiancecube_generator";

		pass_ci.sizeTarget = "irradiance_cube_target";

		pass_ci.bindWriteResource("out_color", "irradiance_cube_target");
		pass_ci.bindReadResource("sampler_color", "CasualDay4K");

		addRenderPass("generate_irradiance_cube", pass_ci);
	}

	{
		FRenderConfigComputeCreateInfo pass_ci{};
		pass_ci.oneTime = true;
		pass_ci.type = EComputeType::eCubemap;
		pass_ci.queue = EQueueTypeFlagBits::eGraphics;
		pass_ci.shader = "lighting:prefilteredmap_generator";

		pass_ci.sizeTarget = "prefiltered_cube_target";

		pass_ci.bindWriteResource("out_color", "prefiltered_cube_target");
		pass_ci.bindReadResource("sampler_color", "CasualDay4K");

		addRenderPass("generate_prefiltered_cube", pass_ci);
	}

	{
		FRenderConfigComputeCreateInfo pass_ci{};
		pass_ci.type = EComputeType::eCulling;
		pass_ci.queue = EQueueTypeFlagBits::eGraphics;
		pass_ci.shader = "culling:cull_and_batch";

		pass_ci.bindReadResource("nodeBuffer", "NodeStorage");
		pass_ci.bindReadResource("frustumSides", "FrustumSides");
		pass_ci.bindReadResource("cullingBuffer", "GeometryCullingInfo");
		pass_ci.bindWriteResource("indirectCommandBuffer", "IndexedDrawIndirectBuffer");

		addRenderPass("gpu_culling", pass_ci);
	}

	{
		FRenderConfigRenderCreateInfo pass_ci{};
		pass_ci.type = ERenderingType::eCascadedShadowMap;
		pass_ci.queue = EQueueTypeFlagBits::eGraphics;
		pass_ci.shader = "lighting:unidirectional_shadow:CASCADED_SHADOW";

		pass_ci.sizeTarget = "cascaded_shadows_target";
		pass_ci.setDepthTarget("cascaded_shadows_target");

		pass_ci.bindReadResource("nodeBuffer", "NodeStorage");
		pass_ci.bindReadResource("materialBuffer", "MaterialStorage");
		pass_ci.bindReadResource("jointsBuffer", "SkinningJoints");
		pass_ci.bindReadResource("cascadedShadowBuffer", "CascadedShadows");
		pass_ci.bindReadResource("batchBuffer", "GeometryBatches");
		pass_ci.addReadResource("IndexedDrawIndirectBuffer");

		addRenderPass("cascaded_shadow_map_pass", pass_ci);
	}

	{
		FRenderConfigRenderCreateInfo pass_ci{};
		pass_ci.type = ERenderingType::eGeometry;
		pass_ci.queue = EQueueTypeFlagBits::eGraphics;
		pass_ci.shader = "visibilitybuffer:write";

		pass_ci.sizeTarget = "zbuffer_depth_target";
		pass_ci.setDepthTarget("zbuffer_depth_target");
		pass_ci.setStencilTarget(pass_ci.depthResource);

		pass_ci.bindReadResource("nodeBuffer", "NodeStorage");
		pass_ci.bindReadResource("jointsBuffer", "SkinningJoints");

		pass_ci.addWriteResource("visbuffer_geometry_target");
		pass_ci.addWriteResource("visbuffer_matobj_target");
		pass_ci.addWriteResource("visbuffer_skinvbo_target");
		pass_ci.addReadResource("IndexedDrawIndirectBuffer");

		addRenderPass("visibility_buffer_write_pass", pass_ci);
	}

	//{
	//	FRenderConfigRenderCreateInfo pass_ci{};
	//	pass_ci.type = ERenderingType::eGeometry;
	//	pass_ci.queue = EQueueTypeFlagBits::eGraphics;
	//	pass_ci.shader = "visibilitybuffer:write";
	//
	//	pass_ci.sizeTarget = "zbuffer_depth_target";
	//	pass_ci.setDepthTarget("zbuffer_depth_target");
	//	pass_ci.setStencilTarget(pass_ci.depthResource);
	//
	//	pass_ci.bindReadResource("nodeBuffer", "NodeStorage");
	//	pass_ci.bindReadResource("jointsBuffer", "SkinningJoints");
	//
	//	pass_ci.addWriteResource("visbuffer_geometry_target");
	//	pass_ci.addWriteResource("visbuffer_matobj_target");
	//	pass_ci.addWriteResource("visbuffer_skinvbo_target");
	//	pass_ci.addReadResource("IndexedDrawIndirectBuffer");
	//
	//	addRenderPass("visibility_buffer_write_pass_2", pass_ci);
	//}

	{
		FRenderConfigComputeCreateInfo pass_ci{};
		pass_ci.type = EComputeType::eGeometry;
		pass_ci.queue = EQueueTypeFlagBits::eGraphics;
		pass_ci.shader = "visibilitybuffer:shade";

		pass_ci.sizeTarget = "final_lighting_target";

		pass_ci.bindWriteResource("write_color", "final_lighting_target");
		pass_ci.bindReadResource("visbuffer_geometry", "visbuffer_geometry_target");
		pass_ci.bindReadResource("visbuffer_matobj", "visbuffer_matobj_target");
		pass_ci.bindReadResource("visbuffer_skinvbo", "visbuffer_skinvbo_target");
		pass_ci.bindReadResource("brdf_lut", "brdf_lut_target");
		pass_ci.bindReadResource("irradiance_cube", "irradiance_cube_target");
		pass_ci.bindReadResource("prefiltered_cube", "prefiltered_cube_target");
		pass_ci.bindReadResource("cascaded_shadowmap", "cascaded_shadows_target");
		pass_ci.bindReadResource("depth_buffer", "zbuffer_depth_target");

		pass_ci.bindReadResource("nodeBuffer", "NodeStorage");
		pass_ci.bindReadResource("materialBuffer", "MaterialStorage");
		pass_ci.bindReadResource("jointsBuffer", "SkinningJoints");
		pass_ci.bindReadResource("directionalLightBuffer", "DirectionalLights");
		pass_ci.bindReadResource("pointLightBuffer", "PointLights");
		pass_ci.bindReadResource("spotLightBuffer", "SpotLights");
		pass_ci.bindReadResource("cascadedShadowBuffer", "CascadedShadows");
		pass_ci.bindReadResource("omniShadowBuffer", "OmniShadows");
		pass_ci.bindReadResource("spotShadowBuffer", "SpotShadows");
		pass_ci.bindReadResource("batchBuffer", "GeometryBatches");

		pass_ci.bindReadResource("indirectCommandBuffer", "IndexedDrawIndirectBuffer");

		addRenderPass("compute_materials_pass", pass_ci);
	}

	{
		FRenderConfigRenderCreateInfo pass_ci{};
		pass_ci.type = ERenderingType::eFullscreen;
		pass_ci.queue = EQueueTypeFlagBits::eGraphics;
		pass_ci.shader = "testing:screenspace";

		pass_ci.sizeTarget = "present_target";

		pass_ci.bindReadResource("input_image", "final_lighting_target");
		pass_ci.addWriteResource("present_target");

		addRenderPass("finalize_image_pass", pass_ci);
	}
}

i64 CRenderGraph::addViewport(const std::string& name, const FViewportCreateInfo& create_info)
{
	auto viewport_index = m_vViewports.size();
	auto& new_viewport = m_vViewports.emplace_back();
	new_viewport.scale = create_info.scale;
	new_viewport.dependencyType = create_info.dependencyType;

	if (create_info.dependencyType == EViewportDependencyType::eOnRenderTarget)
	{
		if (auto found = m_nameToResource.find(create_info.dependsOn); found != m_nameToResource.end())
		{
			auto& resource = m_vRenderResources.at(found->second);
			if(resource.type & ERenderResourceTypeFlagBits::eAllImage)
				new_viewport.dependencyResource = found->second;
		}
	}
	else if (create_info.dependencyType == EViewportDependencyType::eOnViewport)
	{
		if (auto found = m_nameToViewport.find(create_info.dependsOn); found != m_nameToViewport.end())
			new_viewport.dependencyResource = found->second;
	}
	else
	{
		new_viewport.width = create_info.width;
		new_viewport.height = create_info.height;
		new_viewport.depth = create_info.depth;
	}

	m_nameToViewport[name] = viewport_index;
	return viewport_index;
}

void CRenderGraph::setDefaultViewport(const std::string& name)
{
	if (m_defaultViewport != ~0ull)
		log_warning("Default viewport was already set. Setting new one. New default viewport is: {}", name);

	if (auto found = m_nameToViewport.find(name); found != m_nameToViewport.end())
	{
		m_defaultViewport = found->second;
		return;
	}

	log_error("No viewport found with name: {}.", name);
}

const FViewportView& CRenderGraph::getViewportView(const std::string& name) const
{
	static FViewportView invalid{};

	if (auto found = m_nameToViewport.find(name); found != m_nameToViewport.end())
		return m_vViewports.at(found->second);

	return invalid;
}

const FViewportView& CRenderGraph::getViewportView(u64 idx) const
{
	return m_vViewports.at(idx);
}

const FViewportView& CRenderGraph::getDefaultViewportView() const
{
	return m_vViewports.at(m_defaultViewport);
}
const FViewport& CRenderGraph::getViewport(const std::string& name) const
{
	static FViewport invalid{};

	if (auto found = m_nameToViewport.find(name); found != m_nameToViewport.end())
		return m_vViewportBuffer.at(found->second);

	return invalid;
}

const FViewport& CRenderGraph::getViewport(u64 idx) const
{
	return m_vViewportBuffer.at(idx);
}

const FViewport& CRenderGraph::getDefaultViewport() const
{
	return m_vViewportBuffer.at(m_defaultViewport);
}

u64 CRenderGraph::getDefaultViewportIndex() const
{
	return m_defaultViewport;
}

u64 CRenderGraph::addStaticResource(const stl::string& name, const stl::string& path)
{
	auto& render_context = m_pRenderInterface->getRenderContext();

	m_nameToResource[name] = m_vRenderResources.size();

	auto& new_render_resource = m_vRenderResources.emplace_back();
	new_render_resource.type = ERenderResourceTypeFlagBits::eStaticImage;

	FTextureCreateInfo create_info{}; 
	FTextureUploadInfo upload_info{};
	new_render_resource.handle = m_pRenderInterface->loadTexture(path, create_info, upload_info);

	if (new_render_resource.handle == ~0ull)
		new_render_resource.handle = render_context->createTexture(name, create_info, stl::move(upload_info));

	return new_render_resource.handle;
}

u64 CRenderGraph::addRenderResource(const stl::string& name, const FTextureCreateInfo& texture_ci)
{
	auto& render_context = m_pRenderInterface->getRenderContext();
	auto render_extent = render_context->getRenderResolution();
	auto resource_index = m_vRenderResources.size();

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

	auto& new_render_resource = m_vRenderResources.emplace_back();
	new_render_resource.handle = render_context->createRenderTarget(name, texture_create_info);
	new_render_resource.type = (texture_ci.typeFlags & ETextureCreateFlagBits::eStorage) ? ERenderResourceTypeFlagBits::eStorageTarget : ERenderResourceTypeFlagBits::eRenderTarget;
	
	m_nameToResource[name] = resource_index;
	return new_render_resource.handle;
}

u64 CRenderGraph::addRenderResource(const stl::string& name, const FBufferCreateInfo& buffer_ci)
{
	auto& render_context = m_pRenderInterface->getRenderContext();

	m_nameToResource[name] = m_vRenderResources.size();

	auto& new_render_resource = m_vRenderResources.emplace_back();
	new_render_resource.handle = render_context->createBuffer(name, buffer_ci);
	new_render_resource.type = ERenderResourceTypeFlagBits::eBuffer;

	return new_render_resource.handle;
}

void CRenderGraph::setBackbufferResourceName(const stl::string& name)
{
	if (m_backBufferResource != ~0ull)
	{
		log_warning("Backbuffer name is already set. Adding new name association: {}.", name);
		m_nameToResource[name] = m_backBufferResource;
		return;
	}

	auto resource_index = m_vRenderResources.size();

	auto& new_render_resource = m_vRenderResources.emplace_back();
	new_render_resource.type = ERenderResourceTypeFlagBits::eBackbuffer;

	m_backBufferResource = resource_index;
	m_nameToResource[name] = m_backBufferResource;
}

const FRenderResource& CRenderGraph::getRenderResource(const stl::string& name) const
{
	static FRenderResource empty_resource{};
	
	auto found = m_nameToResource.find(name);
	if (found != m_nameToResource.end())
		return m_vRenderResources.at(found->second);

	return empty_resource;
}

const FRenderResource& CRenderGraph::getRenderResource(u64 resource_id) const
{
	return m_vRenderResources.at(resource_id);
}

u64 CRenderGraph::getRenderRersourceId(const stl::string& name) const
{
	auto found = m_nameToResource.find(name);
	if (found != m_nameToResource.end())
		return found->second;

	return ~0ull;
}

u64 CRenderGraph::addRenderPass(const stl::string& name, const FRenderConfigRenderCreateInfo& render_pass_ci)
{
	u64 pass_index = m_vRenderPasses.size();
	m_vPassExecutionQueue.emplace_back(pass_index);

	FRenderPassCreateInfo pass_ci{};
	if (auto found_viewport = m_nameToViewport.find(render_pass_ci.viewport); found_viewport != m_nameToViewport.end())
		pass_ci.viewportIndex = found_viewport->second;
	else
		pass_ci.viewportIndex = m_defaultViewport;
	pass_ci.flipViewport = render_pass_ci.flipViewportState;

	applyRenderPassResources(pass_index, &render_pass_ci, &pass_ci);

	stl::unique_ptr<IRenderPass> new_pass{};
	if (render_pass_ci.type == ERenderingType::eFullscreen)
		new_pass = stl::make_unique<CFullscreenPass>(m_pRenderInterface);
	else if (render_pass_ci.type == ERenderingType::eGeometry)
		new_pass = stl::make_unique<CGeometryPass>(m_pRenderInterface);
	else if (render_pass_ci.type == ERenderingType::eCascadedShadowMap)
		new_pass = stl::make_unique<CCascadedShadowMapPass>(m_pRenderInterface);

	if (!render_pass_ci.depthResource.empty())
		pass_ci.depthTarget = getRenderRersourceId(render_pass_ci.depthResource);

	if (!render_pass_ci.stencilResource.empty())
		pass_ci.depthTarget = getRenderRersourceId(render_pass_ci.stencilResource);

	new_pass->init(pass_ci);
	m_vRenderPasses.emplace_back(stl::move(new_pass));
	m_nameToPass[name] = pass_index;

	return pass_index;
}

u64 CRenderGraph::addRenderPass(const stl::string& name, const FRenderConfigComputeCreateInfo& compute_pass_ci)
{
	u64 pass_index = m_vRenderPasses.size();
	m_vPassExecutionQueue.emplace_back(pass_index);

	FComputePassCreateInfo pass_ci{};
	applyRenderPassResources(pass_index, &compute_pass_ci, &pass_ci);

	stl::unique_ptr<IComputePass> new_pass{};
	if (compute_pass_ci.type == EComputeType::eDefault)
		new_pass = stl::make_unique<CComputePass>(m_pRenderInterface);
	else if (compute_pass_ci.type == EComputeType::eGeometry)
		new_pass = stl::make_unique<CComputeGeometryPass>(m_pRenderInterface);
	else if (compute_pass_ci.type == EComputeType::eCubemap)
		new_pass = stl::make_unique<CComputeCubemapPass>(m_pRenderInterface);
	else if (compute_pass_ci.type == EComputeType::eCulling)
		new_pass = stl::make_unique<CComputeCullingPass>(m_pRenderInterface);

	new_pass->init(pass_ci);
	m_vRenderPasses.emplace_back(stl::move(new_pass));
	m_nameToPass[name] = pass_index;

	return pass_index;
}

template <class _Ty, class _KTy>
struct counter_pair : stl::pair <_Ty, _KTy>
{
	using stl::pair<_Ty, _KTy>::pair;

	bool operator== (const counter_pair& other)
	{
		stl::pair p1 = *this;
		stl::pair p2 = other;

		return p1 == p2;
	}
};

void CRenderGraph::build()
{
	if (m_backBufferResource == ~0ull)
	{
		log_error("Failed to build render graph. Reason: back buffer resource is not set.");
		return;
	}

	auto& backbuffer_resource = m_vRenderResources.at(m_backBufferResource);
	if (!backbuffer_resource.usages.count(ERenderResourceAccessFlagBits::eWrite))
	{
		log_error("Failed to build render graph. Reason: No one writes to back buffer resource.");
		return;
	}

	//m_vPassDependencies.resize(m_vRenderPasses.size());
	//m_vPassMergeDependencies.resize(m_vRenderPasses.size());
	//m_vPassProcessing.resize(m_vRenderPasses.size());
	//
	//for (u64 pass_index : backbuffer_resource.usages[ERenderResourceAccessFlagBits::eWrite])
	//	traverseDependencies(pass_index);
	//
	//stl::reverse(m_vPassExecutionQueue.begin(), m_vPassExecutionQueue.end());
	//filter_pass_queue(m_vPassExecutionQueue);
	//
	//reorderPasses(m_vPassExecutionQueue);
	//
	m_vPassOrder.resize(m_vRenderPasses.size());
	u64 pass_order{ 0ull };
	for (u64 pass_index : m_vPassExecutionQueue)
		m_vPassOrder.at(pass_index) = pass_order++;

	// Generate dependency barriers
	stl::unordered_map<u64, stl::vector<FResourceAccessInfo>> per_pass_resource_access{};
	stl::map<stl::pair<ERenderResourceDependencyBarrierType, u64>, u64> unique_barrier_counter;
	stl::unordered_map<u64, u64> unique_resource_writes{};

	for (u64 src_pass_index : m_vPassExecutionQueue)
	{
		auto src_pass_order = m_vPassOrder.at(src_pass_index);
		auto& src_pass = m_vRenderPasses.at(src_pass_index);
		for (const auto& [resource_group, resource_indices] : src_pass->m_renderResources)
		{
			ERenderResourceAccessFlagBits src_access_flag{};
			ERenderResourceGroupFlagBits group_flag{};
			unpackResourceGroup(resource_group, src_access_flag, group_flag);

			for (u64 resource_index : resource_indices)
			{
				// Write first resource access barriers
				if (src_access_flag == ERenderResourceAccessFlagBits::eWrite && !unique_resource_writes.count(resource_index))
				{
					auto& resource_access_infos = per_pass_resource_access[src_pass_index];
					auto& access_info = resource_access_infos.emplace_back();
					access_info.type = packDependencyBarrier(ERenderResourceAccessFlagBits::eNone, src_access_flag);
					access_info.srcPassIndex = src_pass_index;
					access_info.resource = resource_index;

					unique_resource_writes[resource_index]++;
				}

				auto& render_resource = m_vRenderResources.at(resource_index);
				for (const auto& [dst_access_flag, accessing_passes] : render_resource.usages)
				{
					for (u64 dst_pass_index : accessing_passes)
					{
						auto dst_pass_order = m_vPassOrder.at(dst_pass_index);
						if ((src_pass_index == dst_pass_index) || (src_access_flag == ERenderResourceAccessFlagBits::eRead && dst_access_flag == ERenderResourceAccessFlagBits::eRead) || (dst_pass_order < src_pass_order))
							continue;

						auto barrier_type = packDependencyBarrier(src_access_flag, dst_access_flag);
						
						// Filter repeating barriers
						auto resource_barrier_pair = stl::make_pair(barrier_type, render_resource.handle);
						if (unique_barrier_counter.count(resource_barrier_pair))
							continue;

						auto& resource_access_infos = per_pass_resource_access[dst_pass_index];
						auto& access_info = resource_access_infos.emplace_back();
						access_info.type = barrier_type;
						access_info.srcPassIndex = src_pass_index;
						access_info.dstPassIndex = dst_pass_index;
						access_info.resource = resource_index;

						unique_barrier_counter[resource_barrier_pair]++;
					}
				}
			}
		}
	}

	// Prepare resource barriers
	for (const auto& [pass_index, access_infos] : per_pass_resource_access)
	{
		auto& pass_barriers = m_perPassResourceBarriers[pass_index];
		for (const auto& access_info : access_infos)
		{
			auto& new_barrier = pass_barriers.emplace_back();
			auto& render_resource = m_vRenderResources.at(access_info.resource);

			if (access_info.srcPassIndex != ~0ull)
			{
				auto& src_pass = m_vRenderPasses.at(access_info.srcPassIndex);
				new_barrier.srcPassType = src_pass->type();
			}
			
			if (access_info.dstPassIndex != ~0ull)
			{
				auto& dst_pass = m_vRenderPasses.at(access_info.dstPassIndex);
				new_barrier.dstPassType = dst_pass->type();
			}

			new_barrier.type = access_info.type;
			new_barrier.resource.handle = render_resource.handle;
			new_barrier.resource.type = render_resource.type;
		}
	}

	// Build render passes
	for (u64 pass_index : m_vPassExecutionQueue)
	{
		FPassBuildInfo build_info{};

		auto& resource_barriers = per_pass_resource_access[pass_index];
		for (const auto& access_info : resource_barriers)
		{
			auto& render_resource = m_vRenderResources.at(access_info.resource);
			auto& attachment = build_info.attachments[access_info.resource];
			attachment.loadOp = access_info.type == ERenderResourceDependencyBarrierType::eWriteAfterWrite ? ERenderResourceLoadOp::eLoad : ERenderResourceLoadOp::eClear;
			attachment.storeOp = (render_resource.usages.count(ERenderResourceAccessFlagBits::eRead) || access_info.resource == m_backBufferResource) ? ERenderResourceStoreOp::eStore : ERenderResourceStoreOp::eDontCare;
		}

		auto& pass = m_vRenderPasses.at(pass_index);
		pass->build(build_info);
	}

	//  Prepare viewports
	m_vViewportBuffer.resize(m_vViewports.size());

	return;
}

void CRenderGraph::update(f32 dt)
{
	auto& render_context = m_pRenderInterface->getRenderContext();
	auto& render_packet = m_pRenderInterface->getRenderPacket();

	// TODO: traverse dependencies before updating
	for (u64 idx = 0ull; idx < m_vViewports.size(); ++idx)
	{
		auto& viewport_view = m_vViewports.at(idx);
		auto& viewport = m_vViewportBuffer.at(idx);

		if (viewport_view.dependencyType == EViewportDependencyType::eOnRenderTarget)
		{
			auto& render_resource = getRenderResource(viewport_view.dependencyResource);
			if (render_resource.type == ERenderResourceTypeFlagBits::eRenderTarget)
			{
				auto& resource_sizes = render_context->getTextureInfo(render_resource.handle);
				viewport.size.x = resource_sizes.size.width;
				viewport.size.y = resource_sizes.size.height;
			}
			else if(render_resource.type == ERenderResourceTypeFlagBits::eBackbuffer)
			{
				auto backbuffer_size = render_context->getScreenRenderRect();
				viewport.size.x = backbuffer_size.size.x;
				viewport.size.y = backbuffer_size.size.y;
			}
		}
		else if (viewport_view.dependencyType == EViewportDependencyType::eOnViewport)
		{
			auto& depend_viewport_view = m_vViewports.at(viewport_view.dependencyResource);
			auto& depend_viewport = m_vViewportBuffer.at(viewport_view.dependencyResource);

			viewport.size.x = depend_viewport.size.x;
			viewport.size.y = depend_viewport.size.y;
		}
		else
		{
			viewport.size.x = utl::cast<f32>(viewport_view.width);
			viewport.size.y = utl::cast<f32>(viewport_view.height);
		}

		viewport.size.x *= viewport_view.scale;
		viewport.size.y *= viewport_view.scale;

		viewport.invSize.x = 1.f / viewport.size.x;
		viewport.invSize.y = 1.f / viewport.size.y;

		auto aspect = viewport.size.x / viewport.size.y;
		viewport.projection = glm::perspective(render_packet->camera.fov, aspect, render_packet->camera.nearp, render_packet->camera.farp);
		viewport.invProjection = glm::inverse(viewport.projection);
		viewport.viewProjection = viewport.projection * render_packet->camera.view;
		viewport.invViewProjection = glm::inverse(viewport.viewProjection);
	}

	auto& render_world = m_pRenderInterface->getRenderWorld();
	auto& viewport_buffer = render_world->getDrawBuffer(ERenderBufferIndices::eViewportBuffer);
	viewport_buffer->clear();
 	viewport_buffer->copy(m_vViewportBuffer);
}

void CRenderGraph::execute(f32 dt)
{
	auto& render_context = m_pRenderInterface->getRenderContext();

	for (u64 pass_index : m_vPassExecutionQueue)
	{
		auto& pass = m_vRenderPasses[pass_index];
		auto& resource_barriers = m_perPassResourceBarriers[pass_index];

		if (!pass->canExecute())
			continue;

		render_context->processResourceBarriers(pass->m_queue, resource_barriers);
		pass->execute(dt);
	}

	auto& backbuffer_resource = m_vRenderResources.at(m_backBufferResource);
	stl::vector<FResourceBarrierInfo> backbuffer_barriers{};

	auto& backbuffer_barrier = backbuffer_barriers.emplace_back();
	backbuffer_barrier.resource.handle = backbuffer_resource.handle;
	backbuffer_barrier.resource.type = backbuffer_resource.type;
	backbuffer_barrier.type = packDependencyBarrier(ERenderResourceAccessFlagBits::eWrite, ERenderResourceAccessFlagBits::eRead);
	backbuffer_barrier.srcPassType = EPassType::eRender;
	backbuffer_barrier.dstPassType = EPassType::eNone;

	render_context->processResourceBarriers(EQueueTypeFlagBits::eGraphics, backbuffer_barriers);
}

void CRenderGraph::applyRenderPassResources(u64 pass_index, const FRenderConfigPassCreateInfo* raw_pass_ci, FPassCreateInfo* pass_ci)
{
	pass_ci->queue = raw_pass_ci->queue;
	pass_ci->shader = raw_pass_ci->shader;
	pass_ci->oneTime = raw_pass_ci->oneTime;

	// Detecting size policy for pass
	if (!raw_pass_ci->sizeTarget.empty())
	{
		pass_ci->sizeTarget = getRenderRersourceId(raw_pass_ci->sizeTarget);
		auto& render_resource = getRenderResource(pass_ci->sizeTarget);

		if (render_resource.type & ERenderResourceTypeFlagBits::eAllImage)
			pass_ci->sizePolicy = ERenderingSizeType::eFromRenderTarget;
		else if (render_resource.type & ERenderResourceTypeFlagBits::eBuffer)
			pass_ci->sizePolicy = ERenderingSizeType::eFromBuffer;
	}
	else
		pass_ci->sizePolicy = ERenderingSizeType::eManual;

	// Binded resources for using as input
	for (const auto& [access_flag, bindings] : raw_pass_ci->resourceBindings)
	{
		for (const auto& [binding_point, resource_name] : bindings)
		{
			auto resource_id = getRenderRersourceId(resource_name);
			auto& render_resource = getRenderResource(resource_id);

			ERenderResourceGroupFlagBits group_type{};
			if (render_resource.type & ERenderResourceTypeFlagBits::eBuffer)
				group_type = ERenderResourceGroupFlagBits::eStorage;
			else if (render_resource.type & ERenderResourceTypeFlagBits::eStorageTarget)
				group_type = ERenderResourceGroupFlagBits::eStorageTexture;
			else if (render_resource.type & ERenderResourceTypeFlagBits::eRenderTarget)
				group_type = ERenderResourceGroupFlagBits::eColor;
			else if (render_resource.type & ERenderResourceTypeFlagBits::eBackbuffer)
				group_type = ERenderResourceGroupFlagBits::eBackbuffer;

			// Ignore static image control
			if (!(render_resource.type & ERenderResourceTypeFlagBits::eStaticImage))
			{
				auto resource_key = packResourceGroup(access_flag, group_type);
				pass_ci->resources[resource_key].emplace_back(resource_id);
			}

			pass_ci->bindings.emplace(binding_point, resource_id);
		}
	}

	for (const auto& [access_flag, resources] : raw_pass_ci->otherResources)
	{
		for (const auto& resource_name : resources)
		{
			auto resource_id = getRenderRersourceId(resource_name);
			auto& render_resource = getRenderResource(resource_id);

			ERenderResourceGroupFlagBits group_type{ 0u };
			if (render_resource.type & ERenderResourceTypeFlagBits::eRenderTarget)
				group_type = ERenderResourceGroupFlagBits::eColor;
			else if (render_resource.type & ERenderResourceTypeFlagBits::eStorageTarget)
				group_type = ERenderResourceGroupFlagBits::eStorageTexture;
			else if (render_resource.type & ERenderResourceTypeFlagBits::eBuffer)
				group_type = ERenderResourceGroupFlagBits::eStorage;
			else if (render_resource.type & ERenderResourceTypeFlagBits::eBackbuffer)
				group_type = ERenderResourceGroupFlagBits::eBackbuffer;
			else
				continue;

			auto resource_key = packResourceGroup(access_flag, group_type);
			pass_ci->resources[resource_key].emplace_back(resource_id);
		}
	}

	applyMultiRenderResourceDependencies(pass_index, pass_ci->queue, pass_ci->resources);
}

void CRenderGraph::applySingleRenderResourceDependencies(u64 resource_id, ERenderResourceAccessFlagBits access_type, u64 pass_index, EQueueTypeFlagBits queue_type)
{
	auto& render_resource = m_vRenderResources.at(resource_id);
	render_resource.usages[access_type].emplace(pass_index);
	render_resource.queuesUsed |= queue_type;
}

void CRenderGraph::applyMultiRenderResourceDependencies(u64 pass_index, EQueueTypeFlagBits queue_type, const stl::unordered_map<ERenderResourceGroupType, stl::vector<u64>>& associated_resources)
{
	for (const auto& [type, resources] : associated_resources)
	{
		auto access_type = utl::cast<ERenderResourceAccessFlagBits>(utl::enum_cast(type) & 0xFFFF);
		for (u64 resource_id : resources)
			applySingleRenderResourceDependencies(resource_id, access_type, pass_index, queue_type);
	}
}

void CRenderGraph::traverseDependencies(u64 current_pass_index, u64 stack_count)
{
	m_vPassExecutionQueue.emplace_back(current_pass_index);

	auto& current_pass = m_vRenderPasses.at(current_pass_index);
	for (const auto& [resource_group, resource_indices] : current_pass->m_renderResources)
	{
		ERenderResourceAccessFlagBits access_flag{};
		ERenderResourceGroupFlagBits group_flag{};
		unpackResourceGroup(resource_group, access_flag, group_flag);

		if (access_flag != ERenderResourceAccessFlagBits::eRead)
			continue;

		for (u64 resource_index : resource_indices)
		{
			auto& render_resource = m_vRenderResources.at(resource_index);
			if (!render_resource.usages.count(ERenderResourceAccessFlagBits::eWrite))
				continue;

			auto& writer_passes = render_resource.usages[ERenderResourceAccessFlagBits::eWrite];

			if (stack_count > m_vRenderPasses.size())
				log_error("Cycle detected.");

			for (u64 write_pass_index : writer_passes)
			{
				if (write_pass_index == current_pass_index)
					continue;

				m_vPassDependencies.at(current_pass_index).emplace(write_pass_index);

				if (group_flag == ERenderResourceGroupFlagBits::eColor)
					m_vPassMergeDependencies.at(current_pass_index).emplace(write_pass_index);

				stack_count++;
				traverseDependencies(write_pass_index, stack_count);
				stack_count--;
			}
		}
	}
}

bool CRenderGraph::checkPassDependencies(u64 dst_pass, u64 src_pass)
{
	if (dst_pass == src_pass)
		return true;

	for (const auto& dep : m_vPassDependencies[dst_pass])
	{
		if (checkPassDependencies(dep, src_pass))
			return true;
	}

	return false;
}

void CRenderGraph::reorderPasses(stl::vector<u64>& flattened_passes)
{
	for (const auto& pass_merge_deps : m_vPassMergeDependencies)
	{
		auto pass_index = u64(&pass_merge_deps - m_vPassMergeDependencies.data());
		auto& pass_deps = m_vPassDependencies[pass_index];

		for (const auto& merge_dep : pass_merge_deps)
		{
			for (const auto& dependee : pass_deps)
			{
				// Avoid cycles.
				if (checkPassDependencies(dependee, merge_dep))
					continue;

				if (merge_dep != dependee)
					m_vPassDependencies[merge_dep].insert(dependee);
			}
		}
	}

	if (flattened_passes.size() <= 2ull)
		return;

	stl::vector<u64> unscheduled_passes;
	unscheduled_passes.reserve(m_vRenderPasses.size());
	stl::swap(flattened_passes, unscheduled_passes);

	const auto schedule = [&](unsigned index) 
		{
		// Need to preserve the order of remaining elements.
		flattened_passes.push_back(unscheduled_passes[index]);
		stl::move(unscheduled_passes.begin() + index + 1,
			unscheduled_passes.end(),
			unscheduled_passes.begin() + index);
		unscheduled_passes.pop_back();
		};

	schedule(0);
	while (!unscheduled_passes.empty())
	{
		u64 best_candidate = 0;
		u64 best_overlap_factor = 0;

		for (u64 i = 0ull; i < unscheduled_passes.size(); i++)
		{
			u64 overlap_factor = 0ull;

			// Always try to merge passes if possible on tilers.
			// This might not make sense on desktop however,
			// so we can conditionally enable this path depending on our GPU.
			if (m_vPassMergeDependencies[unscheduled_passes[i]].count(flattened_passes.back()))
				overlap_factor = ~0ull;
			else
			{
				for (auto itr = flattened_passes.rbegin(); itr != flattened_passes.rend(); ++itr)
				{
					if (checkPassDependencies(unscheduled_passes[i], *itr))
						break;
					overlap_factor++;
				}
			}

			if (overlap_factor <= best_overlap_factor)
				continue;

			bool possible_candidate = true;
			for (u64 j = 0; j < i; j++)
			{
				if (checkPassDependencies(unscheduled_passes[i], unscheduled_passes[j]))
				{
					possible_candidate = false;
					break;
				}
			}

			if (!possible_candidate)
				continue;

			best_candidate = i;
			best_overlap_factor = overlap_factor;
		}

		schedule(best_candidate);
	}
}