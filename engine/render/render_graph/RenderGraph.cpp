#include "RenderGraph.h"

#include "render_passes/CascadedShadowMapPass.h"
#include "render_passes/ComputeCubemapPass.h"
#include "render_passes/ComputeCullingPass.h"
#include "render_passes/ComputeGeometryPass.h"
#include "render_passes/ComputePass.h"
#include "render_passes/FullscreenPass.h"
#include "render_passes/GeometryPass.h"

using namespace engine::render;

void CRenderGraph::create()
{
	auto surface_resolution = m_pRenderContext->getRenderResolution();

	{
		addStaticResource("CasualDay4K", "/root/CasualDay4K.ktx2");
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = m_pRenderContext->getApiFormat(FORMAT_RG16_SFLOAT);
		texture_ci.typeFlags = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT | TEXTURE_IMAGE_TYPE_STORAGE_FLAG_BIT;
		texture_ci.sizePolicy = TEXTURE_SIZE_POLICY_FIXED;
		texture_ci.width = texture_ci.height = 1024;
		m_pRenderContext->createRenderTargets("brdf_lut_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = m_pRenderContext->getApiFormat(FORMAT_RGBA32_SFLOAT);
		texture_ci.typeFlags = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT | TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT | TEXTURE_IMAGE_TYPE_STORAGE_FLAG_BIT;
		texture_ci.sizePolicy = TEXTURE_SIZE_POLICY_FIXED;
		texture_ci.width = texture_ci.height = 64;
		m_pRenderContext->createRenderTargets("irradiance_cube_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = m_pRenderContext->getApiFormat(FORMAT_RGBA16_SFLOAT);
		texture_ci.typeFlags = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT | TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT | TEXTURE_IMAGE_TYPE_STORAGE_FLAG_BIT;
		texture_ci.sizePolicy = TEXTURE_SIZE_POLICY_FIXED;
		texture_ci.width = texture_ci.height = 512;
		texture_ci.mipLevels = static_cast<uint32_t>(std::floor(std::log2((std::max)(512, 512)))) + 1u;
		m_pRenderContext->createRenderTargets("prefiltered_cube_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = m_pRenderContext->getDepthFormat(false);
		texture_ci.typeFlags = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT | TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT;
		texture_ci.sizePolicy = TEXTURE_SIZE_POLICY_FIXED;
		texture_ci.width = cascade_shadow_map_resolution;
		texture_ci.height = cascade_shadow_map_resolution;
		texture_ci.arrayLayers = cascade_shadow_map_cascade_count * max_directional_light_shadow_count;
		m_pRenderContext->createRenderTargets("cascaded_shadows_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = m_pRenderContext->getApiFormat(FORMAT_RGBA8_UNORM);
		texture_ci.typeFlags = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT;
		texture_ci.sizePolicy = TEXTURE_SIZE_POLICY_SCALABLE;
		texture_ci.width = surface_resolution.x;
		texture_ci.height = surface_resolution.y;
		m_pRenderContext->createRenderTargets("visbuffer_geometry_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = m_pRenderContext->getApiFormat(FORMAT_RGBA8_UNORM);
		texture_ci.typeFlags = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT;
		texture_ci.sizePolicy = TEXTURE_SIZE_POLICY_SCALABLE;
		texture_ci.width = surface_resolution.x;
		texture_ci.height = surface_resolution.y;
		m_pRenderContext->createRenderTargets("visbuffer_matobj_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = m_pRenderContext->getApiFormat(FORMAT_RGBA8_UNORM);
		texture_ci.typeFlags = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT;
		texture_ci.sizePolicy = TEXTURE_SIZE_POLICY_SCALABLE;
		texture_ci.width = surface_resolution.x;
		texture_ci.height = surface_resolution.y;
		m_pRenderContext->createRenderTargets("visbuffer_skinvbo_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = m_pRenderContext->getDepthFormat(false);
		texture_ci.typeFlags = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT;
		texture_ci.sizePolicy = TEXTURE_SIZE_POLICY_SCALABLE;
		texture_ci.width = surface_resolution.x;
		texture_ci.height = surface_resolution.y;
		m_pRenderContext->createRenderTargets("zbuffer_depth_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = m_pRenderContext->getApiFormat(FORMAT_RGBA16_SFLOAT);
		texture_ci.typeFlags = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT | TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT | TEXTURE_IMAGE_TYPE_STORAGE_FLAG_BIT;
		texture_ci.sizePolicy = TEXTURE_SIZE_POLICY_SCALABLE;
		texture_ci.width = surface_resolution.x;
		texture_ci.height = surface_resolution.y;
		m_pRenderContext->createRenderTargets("final_lighting_target", texture_ci);
	}

	{
		FTextureCreateInfo texture_ci{};
		texture_ci.format = m_pRenderContext->getPresentFormat();
		texture_ci.typeFlags = TEXTURE_IMAGE_TYPE_2D_FLAG_BIT;
		texture_ci.sizePolicy = TEXTURE_SIZE_POLICY_FULL;
		texture_ci.width = surface_resolution.x;
		texture_ci.height = surface_resolution.y;
		texture_ci.isSwapchainTarget = true;
		m_pRenderContext->createRenderTargets("final_lighting_target", texture_ci);
	}


	{
		FRenderConfigComputeCreateInfo pass_ci{};
		pass_ci.oneTime = true;
		pass_ci.type = COMPUTE_DEFAULT_TYPE;
		pass_ci.queue = QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT;
		pass_ci.shader = "lighting:brgflut_generator";

		pass_ci.sizeTarget = "brdf_lut_target";
		pass_ci.bindWriteResource("out_color", "brdf_lut_target");

		addRenderPass("generate_brdflut", pass_ci);
	}

	{
		FRenderConfigComputeCreateInfo pass_ci{};
		pass_ci.oneTime = true;
		pass_ci.type = COMPUTE_DEFAULT_TYPE;
		pass_ci.queue = QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT;
		pass_ci.shader = "lighting:irradiancecube_generator";

		pass_ci.sizeTarget = "irradiance_cube_target";

		pass_ci.bindWriteResource("out_color", "irradiance_cube_target");
		pass_ci.bindReadResource("sampler_color", "CasualDay4K");

		addRenderPass("generate_irradiance_cube", pass_ci);
	}

	{
		FRenderConfigComputeCreateInfo pass_ci{};
		pass_ci.oneTime = true;
		pass_ci.type = COMPUTE_CUBEMAP_TYPE;
		pass_ci.queue = QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT;
		pass_ci.shader = "lighting:prefilteredmap_generator";

		pass_ci.sizeTarget = "prefiltered_cube_target";

		pass_ci.bindWriteResource("out_color", "prefiltered_cube_target");
		pass_ci.bindReadResource("sampler_color", "CasualDay4K");

		addRenderPass("generate_prefiltered_cube", pass_ci);
	}

	{
		FRenderConfigComputeCreateInfo pass_ci{};
		pass_ci.oneTime = true;
		pass_ci.type = COMPUTE_CULLING_TYPE;
		pass_ci.queue = QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT;
		pass_ci.shader = "culling:cull_and_batch";

		addRenderPass("gpu_culling", pass_ci);
	}

	{
		FRenderConfigRenderCreateInfo pass_ci{};
		pass_ci.type = RENDERING_CSM_TYPE;
		pass_ci.queue = QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT;
		pass_ci.shader = "lighting:unidirectional_shadow:CASCADED_SHADOW";

		pass_ci.sizeTarget = "cascaded_shadows_target";
		pass_ci.depthResource = "cascaded_shadows_target";

		addRenderPass("cascaded_shadow_map_pass", pass_ci);
	}

	{
		FRenderConfigRenderCreateInfo pass_ci{};
		pass_ci.type = RENDERING_GEOMETRY_TYPE;
		pass_ci.queue = QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT;
		pass_ci.shader = "visibilitybuffer:write";

		pass_ci.sizeTarget = "zbuffer_depth_target";
		pass_ci.depthResource = "zbuffer_depth_target";
		pass_ci.stencilResource = pass_ci.depthResource;

		pass_ci.addRenderTarget("visbuffer_geometry_target");
		pass_ci.addRenderTarget("visbuffer_matobj_target");
		pass_ci.addRenderTarget("visbuffer_skinvbo_target");

		addRenderPass("visibility_buffer_write_pass", pass_ci);
	}

	{
		FRenderConfigComputeCreateInfo pass_ci{};
		pass_ci.type = COMPUTE_DEFAULT_TYPE;
		pass_ci.queue = QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT;
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

		addRenderPass("compute_materials_pass", pass_ci);
	}

	{
		FRenderConfigRenderCreateInfo pass_ci{};
		pass_ci.type = RENDERING_FULLSCREEN_TYPE;
		pass_ci.queue = QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT;
		pass_ci.shader = "testing:screenspace";
		pass_ci.flipViewport = true;

		pass_ci.bindReadResource("input_image", "final_lighting_target");
		pass_ci.addRenderTarget("present_target");

		addRenderPass("finalize_image_pass", pass_ci);
	}
}

size_t CRenderGraph::addStaticResource(const std::string& name, const std::string& path)
{
	m_nameToResource[name] = m_vRenderResources.size();

	auto& new_render_resource = m_vRenderResources.emplace_back();
	new_render_resource.type = RENDER_RESOURCE_TYPE_STATIC_IMAGE;
	new_render_resource.handle = m_pRenderInterface->createTexture(path);

	return new_render_resource.handle;
}

size_t CRenderGraph::addStaticResource(const std::string& name, const FTextureCreateInfo& texture_ci)
{
	m_nameToResource[name] = m_vRenderResources.size();

	auto& new_render_resource = m_vRenderResources.emplace_back();
	new_render_resource.type = RENDER_RESOURCE_TYPE_STATIC_IMAGE;
	new_render_resource.handle = m_pRenderInterface->createTexture("");

	// TODO: Load teture here

	return new_render_resource.handle;
}

size_t CRenderGraph::addRenderResource(const std::string& name, const FTextureCreateInfo& texture_ci)
{
	m_nameToResource[name] = m_vRenderResources.size();

	auto& new_render_resource = m_vRenderResources.emplace_back();
	new_render_resource.handle = m_pRenderContext->createRenderTargets(name, texture_ci);
	new_render_resource.type = (texture_ci.typeFlags & TEXTURE_IMAGE_TYPE_STORAGE_FLAG_BIT) ? RENDER_RESOURCE_TYPE_STORAGE_TARGET : RENDER_RESOURCE_TYPE_RENDER_TARGET;
	
	return new_render_resource.handle;
}

size_t CRenderGraph::addRenderResource(const std::string& name, const FBufferCreateInfo& buffer_ci)
{
	m_nameToResource[name] = m_vRenderResources.size();

	auto& new_render_resource = m_vRenderResources.emplace_back();
	new_render_resource.handle = m_pRenderContext->createBuffer(name, buffer_ci);
	new_render_resource.type = RENDER_RESOURCE_TYPE_BUFFER;

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

const FRenderResource& CRenderGraph::getRenderResource(size_t resource_id) const
{
	return m_vRenderResources.at(resource_id);
}

size_t CRenderGraph::getRenderRersourceId(const std::string& name) const
{
	auto found = m_nameToResource.find(name);
	if (found != m_nameToResource.end())
		return found->second;

	return ~0ull;
}

size_t CRenderGraph::addRenderPass(const std::string& name, const FRenderConfigRenderCreateInfo& render_pass_ci)
{
	size_t pass_index = m_vRenderPasses.size();

	FRenderPassCreateInfo pass_ci{};
	pass_ci.queue = render_pass_ci.queue;
	pass_ci.shader = render_pass_ci.shader;
	pass_ci.oneTime = render_pass_ci.oneTime;
	pass_ci.flipViewport = render_pass_ci.flipViewport;

	// Detecting size policy for pass
	if (!render_pass_ci.sizeTarget.empty())
	{
		pass_ci.sizeTarget = getRenderRersourceId(render_pass_ci.sizeTarget);
		auto& render_resource = getRenderResource(pass_ci.sizeTarget);

		if (render_resource.type & RENDER_RESOURCE_TYPE_ALL_IMAGE)
			pass_ci.sizePolicy = RENDER_SIZE_TYPE_FROM_RT;
		else if (render_resource.type & RENDER_RESOURCE_TYPE_BUFFER)
			pass_ci.sizePolicy = RENDER_SIZE_TYPE_FROM_BUFFER;
	}
	else
		pass_ci.sizePolicy = RENDER_SIZE_TYPE_MANUAL;

	for (const auto& [access_flag, bindings] : render_pass_ci.resourceBindings)
	{
		for (const auto& [binding_point, resource_name] : bindings)
		{
			auto resource_id = getRenderRersourceId(resource_name);
			auto& render_resource = getRenderResource(resource_id);

			uint16_t group_type{};
			if (render_resource.type & RENDER_RESOURCE_TYPE_BUFFER)
				group_type = RENDER_RESOURCE_GROUP_STORAGE_BIT;
			else if (render_resource.type & RENDER_RESOURCE_TYPE_STORAGE_TARGET)
				group_type = RENDER_RESOURCE_GROUP_STORAGE_TEXTURE_BIT;
			else if (render_resource.type & RENDER_RESOURCE_TYPE_STORAGE_TARGET)
				group_type = RENDER_RESOURCE_GROUP_COLOR_BIT;

			auto resource_key = make_render_group_flag(group_type, access_flag);
			pass_ci.resources[resource_key].emplace_back(resource_id);
			pass_ci.bindings.emplace(binding_point, resource_id);
		}
	}

	for (const auto& resource_name : render_pass_ci.renderTargets)
	{
		auto resource_id = getRenderRersourceId(resource_name);
		auto& render_resource = getRenderResource(resource_id);
		pass_ci.resources[RENDER_RESOURCE_GROUP_COLOR_WRITE].emplace_back(resource_id);
	}

	if (!render_pass_ci.depthResource.empty())
	{
		pass_ci.depthTarget = getRenderRersourceId(render_pass_ci.depthResource);
		applySingleRenderResourceDependencies(pass_ci.depthTarget, RENDER_RESOURCE_ACCESS_WRITE_BIT, pass_index, render_pass_ci.queue);
	}

	if (!render_pass_ci.stencilResource.empty())
	{
		pass_ci.stencilTarget = getRenderRersourceId(render_pass_ci.stencilResource);
		applySingleRenderResourceDependencies(pass_ci.stencilTarget, RENDER_RESOURCE_ACCESS_WRITE_BIT, pass_index, render_pass_ci.queue);
	}

	applyMultiRenderResourceDependencies(pass_index, pass_ci.queue, pass_ci.resources);

	std::unique_ptr<IRenderPass> new_pass{};
	if (render_pass_ci.type == RENDERING_FULLSCREEN_TYPE)
		new_pass = std::make_unique<CFullscreenPass>(m_pRenderInterface, m_pRenderContext, this);
	else if (render_pass_ci.type == RENDERING_GEOMETRY_TYPE)
		new_pass = std::make_unique<CGeometryPass>(m_pRenderInterface, m_pRenderContext, this);
	else if (render_pass_ci.type == RENDERING_CSM_TYPE)
		new_pass = std::make_unique<CCascadedShadowMapPass>(m_pRenderInterface, m_pRenderContext, this);

	new_pass->init(pass_ci);
	m_vRenderPasses.emplace_back(std::move(new_pass));
	m_nameToPass[name] = pass_index;

	return pass_index;
}

size_t CRenderGraph::addRenderPass(const std::string& name, const FRenderConfigComputeCreateInfo& compute_pass_ci)
{
	size_t pass_index = m_vRenderPasses.size();

	FComputePassCreateInfo pass_ci{};
	pass_ci.queue = compute_pass_ci.queue;
	pass_ci.shader = compute_pass_ci.shader;
	pass_ci.oneTime = compute_pass_ci.oneTime;

	// Detecting size policy for pass
	if (!compute_pass_ci.sizeTarget.empty())
	{
		pass_ci.sizeTarget = getRenderRersourceId(compute_pass_ci.sizeTarget);
		auto& render_resource = getRenderResource(pass_ci.sizeTarget);

		if (render_resource.type & RENDER_RESOURCE_TYPE_ALL_IMAGE)
			pass_ci.sizePolicy = RENDER_SIZE_TYPE_FROM_RT;
		else if (render_resource.type & RENDER_RESOURCE_TYPE_BUFFER)
			pass_ci.sizePolicy = RENDER_SIZE_TYPE_FROM_BUFFER;
	}
	else
		pass_ci.sizePolicy = RENDER_SIZE_TYPE_MANUAL;

	for (const auto& [access_flag, bindings] : compute_pass_ci.resourceBindings)
	{
		for (const auto& [binding_point, resource_name] : bindings)
		{
			auto resource_id = getRenderRersourceId(resource_name);
			auto& render_resource = getRenderResource(resource_id);

			uint16_t group_type{};
			if (render_resource.type & RENDER_RESOURCE_TYPE_BUFFER)
				group_type = RENDER_RESOURCE_GROUP_STORAGE_BIT;
			else if (render_resource.type & RENDER_RESOURCE_TYPE_STORAGE_TARGET)
				group_type = RENDER_RESOURCE_GROUP_STORAGE_TEXTURE_BIT;
			else if (render_resource.type & RENDER_RESOURCE_TYPE_STORAGE_TARGET)
				group_type = RENDER_RESOURCE_GROUP_COLOR_BIT;

			auto resource_key = make_render_group_flag(group_type, access_flag);
			pass_ci.resources[resource_key].emplace_back(resource_id);
			pass_ci.bindings.emplace(binding_point, resource_id);
		}
	}

	applyMultiRenderResourceDependencies(pass_index, pass_ci.queue, pass_ci.resources);

	std::unique_ptr<IComputePass> new_pass{};
	if (compute_pass_ci.type == COMPUTE_DEFAULT_TYPE)
		new_pass = std::make_unique<CComputePass>(m_pRenderInterface, m_pRenderContext, this);
	else if (compute_pass_ci.type == COMPUTE_GEOMETRY_TYPE)
		new_pass = std::make_unique<CComputeGeometryPass>(m_pRenderInterface, m_pRenderContext, this);
	else if (compute_pass_ci.type == COMPUTE_CUBEMAP_TYPE)
		new_pass = std::make_unique<CComputeCubemapPass>(m_pRenderInterface, m_pRenderContext, this);
	else if (compute_pass_ci.type == COMPUTE_CULLING_TYPE)
		new_pass = std::make_unique<CComputeCullingPass>(m_pRenderInterface, m_pRenderContext, this);

	new_pass->init(pass_ci);
	m_vRenderPasses.emplace_back(std::move(new_pass));
	m_nameToPass[name] = pass_index;

	return pass_index;
}

void CRenderGraph::bake()
{

}

void CRenderGraph::execute(float dt)
{

}

void CRenderGraph::applySingleRenderResourceDependencies(size_t resource_id, uint16_t access_type, size_t pass_index, uint32_t queue_type)
{
	auto& render_resource = m_vRenderResources.at(resource_id);
	render_resource.usages[access_type].emplace(pass_index);
	render_resource.queuesUsed |= queue_type;
}

void CRenderGraph::applyMultiRenderResourceDependencies(size_t pass_index, uint32_t queue_type, const std::unordered_map<uint32_t, std::vector<size_t>>& associated_resources)
{
	for (const auto& [type, resources] : associated_resources)
	{
		uint16_t access_type = type >> 16;
		for (size_t resource_id : resources)
			applySingleRenderResourceDependencies(resource_id, access_type, pass_index, queue_type);
	}
}