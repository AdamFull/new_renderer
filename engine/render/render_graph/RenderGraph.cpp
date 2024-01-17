#include "RenderGraph.h"

#include "render_passes/CascadedShadowMapPass.h"
#include "render_passes/ComputeCubemapPass.h"
#include "render_passes/ComputeCullingPass.h"
#include "render_passes/ComputeGeometryPass.h"
#include "render_passes/ComputePass.h"
#include "render_passes/FullscreenPass.h"
#include "render_passes/GeometryPass.h"

#include <logger/logger.h>

using namespace engine::render;

void filter_pass_queue(std::vector<u64>& pass_list)
{
	std::unordered_set<u64> seen;

	pass_list.erase(std::remove_if(pass_list.begin(), pass_list.end(),
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
		texture_ci.mipLevels = utl::cast<u32>(std::floor(std::log2((std::max)(512, 512)))) + 1u;
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

	auto resource_index = m_vRenderResources.size();

	auto& new_render_resource = m_vRenderResources.emplace_back();
	new_render_resource.handle = render_context->createRenderTargets(name, texture_ci);
	new_render_resource.type = (texture_ci.typeFlags & ETextureCreateFlagBits::eStorage) ? ERenderResourceTypeFlagBits::eStorageTarget : ERenderResourceTypeFlagBits::eRenderTarget;

	if (m_backBufferResource == ~0ull && texture_ci.isSwapchainTarget)
		m_backBufferResource = resource_index;
	
	m_nameToResource[name] = resource_index;
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

	m_vPassDependencies.resize(m_vRenderPasses.size());
	m_vPassMergeDependencies.resize(m_vRenderPasses.size());

	for (u64 pass_index : backbuffer_resource.usages[ERenderResourceAccessFlagBits::eWrite])
		traverseDependencies(pass_index);

	std::reverse(m_vPassExecutionQueue.begin(), m_vPassExecutionQueue.end());
	filter_pass_queue(m_vPassExecutionQueue);

	for (u64 pass_index : m_vPassExecutionQueue)
	{
		// If c
	}

	return;
}

void CRenderGraph::execute(f32 dt)
{
	for (u64 pass_index : m_vPassExecutionQueue)
	{

	}
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
				if (write_pass_index != current_pass_index)
				{
					m_vPassDependencies.at(current_pass_index).emplace(write_pass_index);

					if(group_flag == ERenderResourceGroupFlagBits::eColor)
						m_vPassMergeDependencies.at(current_pass_index).emplace(write_pass_index);
				}
			}

			for (u64 write_pass_index : writer_passes)
			{
				if (write_pass_index == current_pass_index)
					continue;
				else if (write_pass_index == current_pass_index)
					log_error("Pass depends on itself.");

				traverseDependencies(write_pass_index, stack_count);
			}
		}
	}

	stack_count++;
}

//void CRenderGraph::reorderPasses(std::vector<u64>& flattened_passes)
//{
//	for (auto& merge_passes : m_vPassMergeDependencies)
//	{
//		auto pass_index = u64(&merge_passes - m_vPassMergeDependencies.data());
//		auto& pass_deps = m_vPassDependencies[pass_index];
//
//		for (auto& dependency : merge_passes)
//		{
//			for (auto& dependee : pass_deps)
//			{
//				// Avoid cycles.
//				if (dependsOnPass(dependee, dependency))
//					continue;
//
//				if (dependency != dependee)
//					m_vPassDependencies[dependency].emplace(dependee);
//			}
//		}
//	}
//
//	if (flattened_passes.size() <= 2)
//		return;
//
//	std::vector<u64> unscheduled_passes;
//	unscheduled_passes.reserve(m_vRenderPasses.size());
//	std::swap(flattened_passes, unscheduled_passes);
//
//	const auto schedule = [&](unsigned index) 
//		{
//		flattened_passes.push_back(unscheduled_passes[index]);
//		std::move(unscheduled_passes.begin() + index + 1, unscheduled_passes.end(), unscheduled_passes.begin() + index);
//		unscheduled_passes.pop_back();
//		};
//
//	schedule(0);
//	while (!unscheduled_passes.empty())
//	{
//		// Find the next pass to schedule.
//		// We can pick any pass N, if the pass does not depend on anything left in unscheduled_passes.
//		// unscheduled_passes[0] is always okay as a fallback, so unless we find something better,
//		// we will at least pick that.
//
//		// Ideally, we pick a pass which does not introduce any hard barrier.
//		// A "hard barrier" here is where a pass depends directly on the pass before it forcing something ala vkCmdPipelineBarrier,
//		// we would like to avoid this if possible.
//
//		// Find the pass which has the optimal overlap factor which means the number of passes can be scheduled in-between
//		// the depender, and the dependee.
//
//		unsigned best_candidate = 0;
//		unsigned best_overlap_factor = 0;
//
//		for (unsigned i = 0; i < unscheduled_passes.size(); i++)
//		{
//			unsigned overlap_factor = 0;
//
//			// Always try to merge passes if possible on tilers.
//			// This might not make sense on desktop however,
//			// so we can conditionally enable this path depending on our GPU.
//			if (m_vPassMergeDependencies[unscheduled_passes[i]].count(flattened_passes.back()))
//			{
//				overlap_factor = ~0u;
//			}
//			else
//			{
//				for (auto itr = flattened_passes.rbegin(); itr != flattened_passes.rend(); ++itr)
//				{
//					if (dependsOnPass(unscheduled_passes[i], *itr))
//						break;
//					overlap_factor++;
//				}
//			}
//
//			if (overlap_factor <= best_overlap_factor)
//				continue;
//
//			bool possible_candidate = true;
//			for (unsigned j = 0; j < i; j++)
//			{
//				if (dependsOnPass(unscheduled_passes[i], unscheduled_passes[j]))
//				{
//					possible_candidate = false;
//					break;
//				}
//			}
//
//			if (!possible_candidate)
//				continue;
//
//			best_candidate = i;
//			best_overlap_factor = overlap_factor;
//		}
//
//		schedule(best_candidate);
//	}
//}