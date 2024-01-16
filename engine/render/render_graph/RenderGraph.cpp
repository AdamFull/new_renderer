#include "RenderGraph.h"

#include "render_passes/CascadedShadowMapPass.h"
#include "render_passes/ComputeCubemapPass.h"
#include "render_passes/ComputeCullingPass.h"
#include "render_passes/ComputeGeometryPass.h"
#include "render_passes/ComputePass.h"
#include "render_passes/FullscreenPass.h"
#include "render_passes/GeometryPass.h"

#include <stack>

using namespace engine::render;

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
		texture_ci.sizePolicy = ERenderTargetSizePolicy::eFixed;
		texture_ci.width = texture_ci.height = 1024;
		addRenderResource("brdf_lut_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = render_context->getApiFormat(EApiDataFormat::eRGBA32_SFLOAT);
		texture_ci.typeFlags = ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eCubemap | ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage;
		texture_ci.sizePolicy = ERenderTargetSizePolicy::eFixed;
		texture_ci.width = texture_ci.height = 64;
		addRenderResource("irradiance_cube_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = render_context->getApiFormat(EApiDataFormat::eRGBA16_SFLOAT);
		texture_ci.typeFlags = ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eCubemap | ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage;
		texture_ci.sizePolicy = ERenderTargetSizePolicy::eFixed;
		texture_ci.width = texture_ci.height = 512;
		texture_ci.mipLevels = static_cast<uint32_t>(std::floor(std::log2((std::max)(512, 512)))) + 1u;
		addRenderResource("prefiltered_cube_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = render_context->getDepthFormat(false);
		texture_ci.typeFlags = ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eArray | ETextureCreateFlagBits::eSamplable;
		texture_ci.sizePolicy = ERenderTargetSizePolicy::eFixed;
		texture_ci.width = cascade_shadow_map_resolution;
		texture_ci.height = cascade_shadow_map_resolution;
		texture_ci.arrayLayers = cascade_shadow_map_cascade_count * max_directional_light_shadow_count;
		addRenderResource("cascaded_shadows_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = render_context->getApiFormat(EApiDataFormat::eRGBA8_UNORM);
		texture_ci.typeFlags = ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eSamplable;
		texture_ci.sizePolicy = ERenderTargetSizePolicy::eScalable;
		texture_ci.width = surface_resolution.x;
		texture_ci.height = surface_resolution.y;
		addRenderResource("visbuffer_geometry_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = render_context->getApiFormat(EApiDataFormat::eRGBA8_UNORM);
		texture_ci.typeFlags = ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eSamplable;
		texture_ci.sizePolicy = ERenderTargetSizePolicy::eScalable;
		texture_ci.width = surface_resolution.x;
		texture_ci.height = surface_resolution.y;
		addRenderResource("visbuffer_matobj_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = render_context->getApiFormat(EApiDataFormat::eRGBA8_UNORM);
		texture_ci.typeFlags = ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eSamplable;
		texture_ci.sizePolicy = ERenderTargetSizePolicy::eScalable;
		texture_ci.width = surface_resolution.x;
		texture_ci.height = surface_resolution.y;
		addRenderResource("visbuffer_skinvbo_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = render_context->getDepthFormat(false);
		texture_ci.typeFlags = ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eSamplable;
		texture_ci.sizePolicy = ERenderTargetSizePolicy::eScalable;
		texture_ci.width = surface_resolution.x;
		texture_ci.height = surface_resolution.y;
		addRenderResource("zbuffer_depth_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = render_context->getApiFormat(EApiDataFormat::eRGBA16_SFLOAT);
		texture_ci.typeFlags = ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage;
		texture_ci.sizePolicy = ERenderTargetSizePolicy::eScalable;
		texture_ci.width = surface_resolution.x;
		texture_ci.height = surface_resolution.y;
		addRenderResource("final_lighting_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = render_context->getPresentFormat();
		texture_ci.typeFlags = ETextureCreateFlagBits::e2D;
		texture_ci.sizePolicy = ERenderTargetSizePolicy::eFull;
		texture_ci.width = surface_resolution.x;
		texture_ci.height = surface_resolution.y;
		texture_ci.isSwapchainTarget = true;
		addRenderResource("present_target", texture_ci);
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

	{
		FRenderConfigComputeCreateInfo pass_ci{};
		pass_ci.type = EComputeType::eDefault;
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
		pass_ci.flipViewport = true;

		pass_ci.bindReadResource("input_image", "final_lighting_target");
		pass_ci.addWriteResource("present_target");

		addRenderPass("finalize_image_pass", pass_ci);
	}
}

u64 CRenderGraph::addStaticResource(const std::string& name, const std::string& path)
{
	auto& render_context = m_pRenderInterface->getRenderContext();

	m_nameToResource[name] = m_vRenderResources.size();

	auto& new_render_resource = m_vRenderResources.emplace_back();
	new_render_resource.type = ERenderResourceTypeFlagBits::eStaticImage;

	FTextureCreateInfo create_info{}; 
	FTextureUploadInfo upload_info{};
	new_render_resource.handle = m_pRenderInterface->loadTexture(path, create_info, upload_info);

	if (new_render_resource.handle != ~0ull)
		new_render_resource.handle = render_context->createTexture(name, create_info, std::move(upload_info));

	return new_render_resource.handle;
}

u64 CRenderGraph::addRenderResource(const std::string& name, const FTextureCreateInfo& texture_ci)
{
	auto& render_context = m_pRenderInterface->getRenderContext();

	m_nameToResource[name] = m_vRenderResources.size();

	auto& new_render_resource = m_vRenderResources.emplace_back();
	new_render_resource.handle = render_context->createRenderTargets(name, texture_ci);
	new_render_resource.type = (texture_ci.typeFlags & ETextureCreateFlagBits::eStorage) ? ERenderResourceTypeFlagBits::eStorageTarget : ERenderResourceTypeFlagBits::eRenderTarget;
	
	return new_render_resource.handle;
}

u64 CRenderGraph::addRenderResource(const std::string& name, const FBufferCreateInfo& buffer_ci)
{
	auto& render_context = m_pRenderInterface->getRenderContext();

	m_nameToResource[name] = m_vRenderResources.size();

	auto& new_render_resource = m_vRenderResources.emplace_back();
	new_render_resource.handle = render_context->createBuffer(name, buffer_ci);
	new_render_resource.type = ERenderResourceTypeFlagBits::eBuffer;

	return new_render_resource.handle;
}

const FRenderResource& CRenderGraph::getRenderResource(const std::string& name) const
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

u64 CRenderGraph::getRenderRersourceId(const std::string& name) const
{
	auto found = m_nameToResource.find(name);
	if (found != m_nameToResource.end())
		return found->second;

	return ~0ull;
}

u64 CRenderGraph::addRenderPass(const std::string& name, const FRenderConfigRenderCreateInfo& render_pass_ci)
{
	u64 pass_index = m_vRenderPasses.size();

	FRenderPassCreateInfo pass_ci{};
	pass_ci.flipViewport = render_pass_ci.flipViewport;

	applyRenderPassResources(pass_index, &render_pass_ci, &pass_ci);

	std::unique_ptr<IRenderPass> new_pass{};
	if (render_pass_ci.type == ERenderingType::eFullscreen)
		new_pass = std::make_unique<CFullscreenPass>(m_pRenderInterface);
	else if (render_pass_ci.type == ERenderingType::eGeometry)
		new_pass = std::make_unique<CGeometryPass>(m_pRenderInterface);
	else if (render_pass_ci.type == ERenderingType::eCascadedShadowMap)
		new_pass = std::make_unique<CCascadedShadowMapPass>(m_pRenderInterface);

	new_pass->init(pass_ci);
	m_vRenderPasses.emplace_back(std::move(new_pass));
	m_nameToPass[name] = pass_index;

	return pass_index;
}

u64 CRenderGraph::addRenderPass(const std::string& name, const FRenderConfigComputeCreateInfo& compute_pass_ci)
{
	u64 pass_index = m_vRenderPasses.size();

	FComputePassCreateInfo pass_ci{};
	applyRenderPassResources(pass_index, &compute_pass_ci, &pass_ci);

	std::unique_ptr<IComputePass> new_pass{};
	if (compute_pass_ci.type == EComputeType::eDefault)
		new_pass = std::make_unique<CComputePass>(m_pRenderInterface);
	else if (compute_pass_ci.type == EComputeType::eGeometry)
		new_pass = std::make_unique<CComputeGeometryPass>(m_pRenderInterface);
	else if (compute_pass_ci.type == EComputeType::eCubemap)
		new_pass = std::make_unique<CComputeCubemapPass>(m_pRenderInterface);
	else if (compute_pass_ci.type == EComputeType::eCulling)
		new_pass = std::make_unique<CComputeCullingPass>(m_pRenderInterface);

	new_pass->init(pass_ci);
	m_vRenderPasses.emplace_back(std::move(new_pass));
	m_nameToPass[name] = pass_index;

	return pass_index;
}

void CRenderGraph::bake()
{
	//struct FPassResourceDependency
	//{
	//	u64 srcPassIndex{ ~0ull };
	//	uint16_t srcAccessType{};
	//
	//	u64 dstPassIndex{ ~0ull };
	//	uint16_t dstAccessType{};
	//};
	//
	//struct FPassResourceBarrier
	//{
	//	uint32_t barrierType{ 0u };
	//
	//	uint32_t resourceType{ 0u };
	//	u64 resourceHandle{ ~0ull };
	//};
	//
	//struct FPassBarriers
	//{
	//	std::vector<FPassResourceBarrier> before{};
	//	std::vector<FPassResourceBarrier> after{};
	//};
	//
	//std::unordered_map<u64, std::unordered_map<uint16_t, u64>> resource_usage_counter{};
	//std::unordered_map<u64, std::vector<FPassResourceDependency>> dependencies{};
	//for (u64 pass_index = 0ull; pass_index < m_vRenderPasses.size(); ++pass_index)
	//{
	//	auto& pass = m_vRenderPasses.at(pass_index);
	//	for (const auto& [resource_mask, resource_ids] : pass->m_renderResources)
	//	{
	//		uint16_t access_mask{};
	//		uint16_t group_mask{};
	//		unpackUint16x2(resource_mask, access_mask, group_mask);
	//
	//		for (u64 resource_id : resource_ids)
	//		{
	//			auto& resource_dependencies = dependencies[resource_id];
	//			auto& usage_counter = resource_usage_counter[resource_id][access_mask];
	//
	//			if (resource_dependencies.empty())
	//			{
	//				auto& resource_dependency = resource_dependencies.emplace_back();
	//				if ((access_mask & ERenderResourceAccessFlagBits::eWrite) && usage_counter == 0ull)
	//				{
	//					resource_dependency.dstPassIndex = pass_index;
	//					resource_dependency.dstAccessType = access_mask;
	//				}
	//				else
	//				{
	//					resource_dependency.srcPassIndex = pass_index;
	//					resource_dependency.srcAccessType = access_mask;
	//				}
	//			}
	//			else
	//			{
	//				auto& last_dependency = resource_dependencies.back();
	//				if (last_dependency.srcPassIndex != ~0ull && (last_dependency.srcAccessType != access_mask || (access_mask & ERenderResourceAccessFlagBits::eWrite)))
	//				{
	//					auto& resource_dependency = resource_dependencies.back();
	//					resource_dependency.dstPassIndex = pass_index;
	//					resource_dependency.dstAccessType = access_mask;
	//				}
	//				else
	//				{
	//					auto& resource_dependency = resource_dependencies.emplace_back();
	//					resource_dependency.srcPassIndex = pass_index;
	//					resource_dependency.srcAccessType = access_mask;
	//				}
	//			}
	//		}
	//	}
	//}
	//
	//// NOTE: Generating per pass barriers. 
	//std::unordered_map<u64, std::vector<FPassResourceBarrier>> per_pass_barriers{};
	//for (const auto& [resource_id, resource_deps] : dependencies)
	//{
	//	auto& render_resource = m_vRenderResources.at(resource_id);
	//	for (const auto& dependency : resource_deps)
	//	{
	//		u64 current_pass{ ~0ull };
	//
	//		if (dependency.dstPassIndex == ~0ull)
	//		{
	//			if (dependency.srcAccessType == ERenderResourceAccessFlagBits::eRead)
	//				continue;
	//
	//			current_pass = dependency.srcPassIndex;
	//		}
	//		else
	//			current_pass = dependency.dstPassIndex;
	//
	//		auto& barrier = per_pass_barriers[current_pass].emplace_back();
	//		barrier.barrierType = packUint16x2(dependency.srcAccessType, dependency.dstAccessType);
	//		barrier.resourceType = render_resource.type;
	//		barrier.resourceHandle = render_resource.handle;
	//	}
	//}

	std::unordered_map<u64, std::unordered_set<u64>> dependencies;
	for (u64 pass_index = 0; pass_index < m_vRenderPasses.size(); ++pass_index)
	{
		auto& pass = m_vRenderPasses[pass_index];
		for (const auto& [resource_group, resource_ids] : pass->m_renderResources)
		{
			ERenderResourceAccessFlagBits access_mask{}; ERenderResourceGroupFlagBits group_mask{};
			unpackResourceGroup(resource_group, access_mask, group_mask);

			for (auto resource_id : resource_ids)
			{
				auto& resource = m_vRenderResources[resource_id];
				for (auto& [resource_access_mask, resource_use_passes] : resource.usages)
				{
					if (access_mask & ERenderResourceAccessFlagBits::eRead && resource_access_mask & ERenderResourceAccessFlagBits::eWrite)
					{
						for (u64 resource_write_pass : resource_use_passes)
							dependencies[pass_index].emplace(resource_write_pass);
					}
				}
			}
		}
	}

	return;
}

void CRenderGraph::execute(float dt)
{

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
			else
				continue;

			auto resource_key = packResourceGroup(access_flag, group_type);
			pass_ci->resources[resource_key].emplace_back(resource_id);
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

void CRenderGraph::applyMultiRenderResourceDependencies(u64 pass_index, EQueueTypeFlagBits queue_type, const std::unordered_map<ERenderResourceGroupType, std::vector<u64>>& associated_resources)
{
	for (const auto& [type, resources] : associated_resources)
	{
		auto access_type = utl::cast<ERenderResourceAccessFlagBits>(utl::enum_cast(type) & 0xFFFF);
		for (u64 resource_id : resources)
			applySingleRenderResourceDependencies(resource_id, access_type, pass_index, queue_type);
	}
}