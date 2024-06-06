#include "RenderGraph.h"

#include <logger/logger.h>
#include <glm/gtc/matrix_transform.hpp>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>

#include "render_passes/CascadedShadowMapPass.h"
#include "render_passes/DirectionalShadowMapPass.h"
#include "render_passes/OmniShadowMapPass.h"
#include "render_passes/ComputeCubemapPass.h"
#include "render_passes/ComputeCullingPass.h"
#include "render_passes/ComputeGeometryPass.h"
#include "render_passes/ComputePass.h"
#include "render_passes/FullscreenPass.h"
#include "render_passes/GeometryPass.h"
#include "render_passes/ClearPass.h"
#include "render_passes/PostProcessPass.h"
#include "render_passes/ImGuiPass.h"

#include "render_passes/BeginDebugMarker.h"
#include "render_passes/EndDebugMarker.h"

#include "ScopedDebugMarker.h"

#include <engine/VariableBlock.h>
#include <engine/Variables.h>

using namespace engine;
using namespace engine::render;

#define SWAP_COUNT(in, out) std::swap(in, out); ++pass_count;

struct pair_hash
{
	template <class _T1, class _T2>
	std::size_t operator()(const std::pair<_T1, _T2>& pair) const 
	{
		std::size_t hash1 = std::hash<_T1>{}(pair.first);
		std::size_t hash2 = std::hash<_T2>{}(pair.second);
		return hash1 ^ hash2;
	}
};

struct tmp_access_info
{
	ERenderResourceAccessFlagBits access{};
	utl::vector<ERenderResourceAccessFlagBits> all_time_access{};
	EPassType pass_type{};
	FResourceBarrierInfo* last_barrier{ nullptr };
};

bool is_x_only_access(const FRenderResource& resource, ERenderResourceAccessFlagBits required_access)
{
	for (const auto& [access_flag, accessing_passes] : resource.usages)
	{
		if (access_flag != required_access)
			return false;
	}

	return true;
}

bool is_last_pass_access(const FRenderResource& resource, const std::unordered_set<u64>& accessed_passes)
{
	for (const auto& [access_flag, accessing_passes] : resource.usages)
	{
		for (const auto& pass_index : accessing_passes)
		{
			if (auto found = accessed_passes.find(pass_index); found == accessed_passes.end())
				return false;

		}
	}

	return true;
}

// Checks is all next passes is read only
bool is_ro_access_only_remain(const FRenderResource& resource, u64 current_pass)
{
	for (const auto& [access_flag, accessing_passes] : resource.usages)
	{
		if (access_flag == ERenderResourceAccessFlagBits::eRead)
			continue;

		for (const auto& pass_index : accessing_passes)
		{
			if (pass_index > current_pass)
				return false;
		}
	}

	return true;
}

bool was_accessed(const tmp_access_info& access_info, ERenderResourceAccessFlagBits access_flag)
{
	auto found = std::find_if(access_info.all_time_access.begin(), access_info.all_time_access.end(),
		[access_flag](const ERenderResourceAccessFlagBits& access) {return access == access_flag; });

	return found != access_info.all_time_access.end();
}

void filter_pass_queue(utl::vector<u64>& pass_list)
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

FTextureCreateInfo CRenderGraph::MakeTextureCI(EApiDataFormat format, ETextureCreateFlags flags)
{
	FTextureCreateInfo target_ci{};
	target_ci.format = format;
	target_ci.flags = flags | ETextureCreateFlagBits::eRenderTarget;
	return target_ci;
}

FTextureCreateInfo CRenderGraph::MakeTexture2DCI(EApiDataFormat format, ETextureCreateFlags flags)
{
	return MakeTextureCI(format, flags | ETextureCreateFlagBits::e2D);
}

FTextureCreateInfo CRenderGraph::MakeTexture2DArrayCI(EApiDataFormat format, ETextureCreateFlags flags)
{
	return MakeTextureCI(format, flags | ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eArray);
}

FTextureCreateInfo CRenderGraph::MakeTexture3DCI(EApiDataFormat format, ETextureCreateFlags flags)
{
	return MakeTextureCI(format, flags | ETextureCreateFlagBits::e3D);
}

FTextureCreateInfo CRenderGraph::MakeTexture3DArrayCI(EApiDataFormat format, ETextureCreateFlags flags)
{
	return MakeTextureCI(format, flags | ETextureCreateFlagBits::e3D | ETextureCreateFlagBits::eArray);
}

FTextureCreateInfo CRenderGraph::MakeTextureCubeCI(EApiDataFormat format, ETextureCreateFlags flags)
{
	return MakeTextureCI(format, flags | ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eCubemap);
}

FTextureCreateInfo CRenderGraph::MakeTextureCubeArrayCI(EApiDataFormat format, ETextureCreateFlags flags)
{
	return MakeTextureCI(format, flags | ETextureCreateFlagBits::e2D | ETextureCreateFlagBits::eCubemap | ETextureCreateFlagBits::eArray);
}

FRenderConfigComputeCreateInfo makeComputePassCI(EComputeType type, EQueueTypeFlagBits queue, const std::string& shader)
{
	FRenderConfigComputeCreateInfo pass_ci{};
	pass_ci.type = type;
	pass_ci.queue = queue;
	pass_ci.shader = shader;

	return pass_ci;
}

FRenderConfigRenderCreateInfo makeGraphicsPassCI(ERenderingType type, EQueueTypeFlagBits queue, const std::string& shader)
{
	FRenderConfigRenderCreateInfo pass_ci{};
	pass_ci.type = type;
	pass_ci.queue = queue;
	pass_ci.shader = shader;

	return pass_ci;
}

bool parseRenderConfig(xercesc::XercesDOMParser* parser, const xercesc::MemBufInputSource& xmlMemBuf)
{
	try
	{
		parser->parse(xmlMemBuf);
		log_debug("[Render Graph] Render config validation successful.");
	}
	catch (const xercesc::XMLException& e)
	{
		char* message = xercesc::XMLString::transcode(e.getMessage());
		log_error("[Render Graph] Render config validation error: {}", message);
		xercesc::XMLString::release(&message);
		return false;
	}
	catch (const xercesc::SAXParseException& e)
	{
		char* message = xercesc::XMLString::transcode(e.getMessage());
		log_error("[Render Graph] Render config validation error: {}", message);
		xercesc::XMLString::release(&message);
		return false;
	}

	return true;
}

std::unique_ptr<IVariable> parseVariable(const char* name, xercesc::DOMElement* var)
{
	std::unique_ptr<IVariable> variable{};

	char* dtype = xercesc::XMLString::transcode(var->getAttribute(xercesc::XMLString::transcode("dtype")));
	char* value = xercesc::XMLString::transcode(var->getTextContent());

	if (!strcmp(dtype, "bool"))
	{
		variable = std::make_unique<CBoolVariable>(FNumericScalarInitialData<bool>{});
	}

	xercesc::XMLString::release(&value);
	xercesc::XMLString::release(&dtype);

	return variable;
}

std::unique_ptr<IVariableBlock> parseVariableBlock(const char* name, xercesc::DOMElement* varBlock)
{
	std::unique_ptr<IVariableBlock> block = std::make_unique<CVariableBlock>();

	for (xercesc::DOMElement* child = varBlock->getFirstElementChild(); child != nullptr; child = child->getNextElementSibling())
	{
		if (xercesc::XMLString::compareString(child->getTagName(), xercesc::XMLString::transcode("Var")) == 0)
		{
			char* name = xercesc::XMLString::transcode(child->getAttribute(xercesc::XMLString::transcode("name")));
			block->AddVariable(name, parseVariable(name, child));
			xercesc::XMLString::release(&name);
		}
		else if (xercesc::XMLString::compareString(child->getTagName(), xercesc::XMLString::transcode("VarBlock")) == 0)
		{
			char* name = xercesc::XMLString::transcode(child->getAttribute(xercesc::XMLString::transcode("name")));
			block->AddBlock(name, parseVariableBlock(name, child));
			xercesc::XMLString::release(&name);
		}
	}

	return block;
}


void CRenderGraph::Create(const utl::vector<u8>& config, const utl::vector<u8>& schema)
{
	//m_pVariableBlock = std::make_unique<CVariableBlock>();
	//
	//xercesc::XMLPlatformUtils::Initialize();
	//
	//xercesc::MemBufInputSource xmlMemBuf(config.data(), config.size(), "xmlBuffer", false);
	//xercesc::MemBufInputSource xsdMemBuf(schema.data(), schema.size(), "xsdBuffer", false);
	//
	//xercesc::XercesDOMParser* parser = new xercesc::XercesDOMParser();
	//
	//xercesc::ErrorHandler* errHandler = (xercesc::ErrorHandler*) new xercesc::HandlerBase();
	//parser->setErrorHandler(errHandler);
	//
	//parser->loadGrammar(xsdMemBuf, xercesc::Grammar::SchemaGrammarType, true);
	//parser->setValidationScheme(xercesc::XercesDOMParser::Val_Always);
	//parser->setDoNamespaces(true);
	//parser->setDoSchema(true);
	//parser->setValidationSchemaFullChecking(true);
	//parser->useCachedGrammarInParse(true);
	//
	//// Parse document with validation
	//if (!parseRenderConfig(parser, xmlMemBuf))
	//{
	//	delete parser;
	//	delete errHandler;
	//	xercesc::XMLPlatformUtils::Terminate();
	//	log_error("[Render Graph] Failed to parse render config.");
	//	return;
	//}
	//
	//// Begin parsing
	//xercesc::DOMDocument* doc = parser->getDocument();
	//
	//xercesc::DOMElement* renderConfig = doc->getDocumentElement();
	//if (!renderConfig || xercesc::XMLString::compareString(renderConfig->getTagName(), xercesc::XMLString::transcode("RenderConfig")) != 0) 
	//{
	//	log_error("[Render Graph] Failed to parse render config. Invalid dom element.");
	//	return;
	//}
	//
	//xercesc::DOMElement* renderVars = renderConfig->getFirstElementChild();
	//while (renderVars) 
	//{
	//	if (xercesc::XMLString::compareString(renderVars->getTagName(), xercesc::XMLString::transcode("RenderVars")) == 0)
	//	{
	//		for (xercesc::DOMElement* child = renderVars->getFirstElementChild(); child != nullptr; child = child->getNextElementSibling())
	//		{
	//			if (xercesc::XMLString::compareString(child->getTagName(), xercesc::XMLString::transcode("Var")) == 0)
	//			{
	//				char* name = xercesc::XMLString::transcode(child->getAttribute(xercesc::XMLString::transcode("name")));
	//				m_pVariableBlock->AddVariable(name, parseVariable(name, child));
	//				xercesc::XMLString::release(&name);
	//			}
	//			else if (xercesc::XMLString::compareString(child->getTagName(), xercesc::XMLString::transcode("VarBlock")) == 0)
	//			{
	//				char* name = xercesc::XMLString::transcode(child->getAttribute(xercesc::XMLString::transcode("name")));
	//				m_pVariableBlock->AddBlock(name, parseVariableBlock(name, child));
	//				xercesc::XMLString::release(&name);
	//			}
	//		}
	//	}
	//	renderVars = renderVars->getNextElementSibling();
	//}
	//
	//delete parser;
	//delete errHandler;
	//xercesc::XMLPlatformUtils::Terminate();

	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto surface_resolution = render_context->GetRenderResolution();
	
	bool use_low_precision_velocity{ true };
	
	bool enable_fxaa{ false };
	bool enable_taa{ true };
	bool motion_blur_enabled{ false };
	bool depth_of_field_enabled{ true };
	bool chromatic_aberration_enabled{ false };
	
	//Bloom settings
	bool bloom_enabled{ true };
	bool bloom_use_brightness{ true };
	int bloom_blur_depth{ 7 };
	
	// Loading test cubemap
	{
		AddStaticResource("CasualDay4K", "/root/embed/rendering/textures/CasualDay4K.ktx2");
		AddStaticResource("LensDirtMask", "/root/embed/rendering/textures/DirtMaskTextureExample.png");
	}
	
	// GI targets
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eRG16_SFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		texture_ci.SetFixedSize2D(1024);
		AddRenderResource("brdf_lut_target", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTextureCubeCI(EApiDataFormat::eRGBA32_SFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		texture_ci.SetFixedSize2D(64);
		AddRenderResource("irradiance_cube_target", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTextureCubeCI(EApiDataFormat::eRGBA16_SFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		texture_ci.SetFixedSize2D(512);
		texture_ci.EnableMipMap();
		AddRenderResource("prefiltered_cube_target", texture_ci);
	}
	
	// Shadow targets
	{
		FTextureCreateInfo texture_ci = MakeTexture2DArrayCI(EApiDataFormat::eD32SFLOAT, ETextureCreateFlagBits::eSamplable);
		texture_ci.SetFixedSize2D(cascade_shadow_map_resolution);
		texture_ci.arrayLayers = cascade_shadow_map_cascade_count * max_directional_light_shadow_count;
		AddRenderResource("cascaded_shadows_target", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTexture2DArrayCI(EApiDataFormat::eD16UNORM, ETextureCreateFlagBits::eSamplable);
		texture_ci.SetFixedSize2D(spot_light_shadow_map_resolution);
		texture_ci.arrayLayers = max_spot_light_shadow_count;
		AddRenderResource("directional_shadows_target", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTextureCubeArrayCI(EApiDataFormat::eD16UNORM, ETextureCreateFlagBits::eSamplable);
		texture_ci.SetFixedSize2D(point_light_shadow_map_resolution);
		texture_ci.arrayLayers = max_point_light_shadow_count;
		AddRenderResource("omni_shadows_target", texture_ci);
	}
	
	// Visibility buffer targets
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eRGBA8_UNORM, ETextureCreateFlagBits::eSamplable);
		AddRenderResource("visbuffer_geometry_target", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eRGBA8_UNORM, ETextureCreateFlagBits::eSamplable);
		AddRenderResource("visbuffer_vboinst_target", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eD32SFLOAT, ETextureCreateFlagBits::eSamplable);
		AddRenderResource("zbuffer_depth_target", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(use_low_precision_velocity ? EApiDataFormat::eRG8_UNORM : EApiDataFormat::eRG16_SFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		AddRenderResource("motion_vectors_target", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eR11G11B10_UFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		AddRenderResource("hdr0", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eR11G11B10_UFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		AddRenderResource("hdr1", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eR11G11B10_UFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		AddRenderResource("hdr0_history", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eR11G11B10_UFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		texture_ci.SetSizeScale(0.5f);
		AddRenderResource("hdr0_div2", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eR11G11B10_UFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		texture_ci.SetSizeScale(0.5f);
		texture_ci.mipLevels = bloom_blur_depth;
		AddRenderResource("bloom_target", texture_ci);
	}
	
	// Eye adaptation targets
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eR32_SFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		texture_ci.SetFixedSize2D(1);
		AddRenderResource("avg_brightness_target", texture_ci);
	}
	
	{
		FBufferCreateInfo buffer_ci{};
		buffer_ci.flags = EBufferUsageFlagBits::eStorageBuffer;
		buffer_ci.size = 4;
		buffer_ci.count = 256;
		AddRenderResource("histogram_buffer", buffer_ci);
	}
	
	// Present image targets
	SetBackbufferResourceName("present_target");
	
	
	{
		FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "lighting:brgflut_generator");
		pass_ci.oneTime = true;
	
		pass_ci.AddWorkgroupSizeByRT("brdf_lut_target", true);
		pass_ci.BindWriteResource("out_color", "brdf_lut_target");
	
		AddRenderPass("generate_brdflut", pass_ci);
	}
	
	{
		FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "lighting:irradiancecube_generator");
		pass_ci.oneTime = true;
	
		pass_ci.AddWorkgroupSizeByRT("irradiance_cube_target", true);
		pass_ci.BindWriteResource("out_color", "irradiance_cube_target");
		pass_ci.BindReadResource("sampler_color", "CasualDay4K");
	
		AddRenderPass("generate_irradiance_cube", pass_ci);
	}
	
	{
		FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eCubemap, EQueueTypeFlagBits::eGraphics, "lighting:prefilteredmap_generator");
		pass_ci.oneTime = true;
	
		pass_ci.AddWorkgroupSizeByRT("prefiltered_cube_target", true);
		pass_ci.BindWriteResource("out_color", "prefiltered_cube_target");
		pass_ci.BindReadResource("sampler_color", "CasualDay4K");
	
		AddRenderPass("generate_prefiltered_cube", pass_ci);
	}
	
	// Culling(culling is temporary)
	{
		FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "culling:clear_counters");
	
		pass_ci.AddWorkgroupSizeByBuffer("IndirectCountBuffer", true);
		pass_ci.AddWriteResource("IndirectCountBuffer");
	
		AddRenderPass("clear_counters_pass", pass_ci);
	}
	
	{
		FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eCulling, EQueueTypeFlagBits::eGraphics, "culling:cull_and_batch");
	
		pass_ci.AddWriteResource("GeometryBatches");
		pass_ci.AddWriteResource("IndexedDrawIndirectBuffer");
		pass_ci.AddWriteResource("IndirectCountBuffer");
	
		AddRenderPass("gpu_culling_pass", pass_ci);
	}
	
	// Shadow passes
	{
		FRenderConfigRenderCreateInfo pass_ci = makeGraphicsPassCI(ERenderingType::eCascadedShadowMap, EQueueTypeFlagBits::eGraphics, "lighting:unidirectional_shadow:CASCADED_SHADOW");
		pass_ci.SetDepthTarget("cascaded_shadows_target");
	
		pass_ci.AddReadResource("GeometryBatches");
		pass_ci.AddReadResource("IndexedDrawIndirectBuffer");
		pass_ci.AddReadResource("IndirectCountBuffer");
	
		AddRenderPass("cascaded_shadow_map_pass", pass_ci);
	}
	
	{
		FRenderConfigRenderCreateInfo pass_ci = makeGraphicsPassCI(ERenderingType::eDirectionalShadowMap, EQueueTypeFlagBits::eGraphics, "lighting:unidirectional_shadow");
		pass_ci.SetDepthTarget("directional_shadows_target");
	
		pass_ci.AddReadResource("GeometryBatches");
		pass_ci.AddReadResource("IndexedDrawIndirectBuffer");
		pass_ci.AddReadResource("IndirectCountBuffer");
	
		AddRenderPass("spot_shadow_map_pass", pass_ci);
	}
	
	{
		FRenderConfigRenderCreateInfo pass_ci = makeGraphicsPassCI(ERenderingType::eOmniShadowMap, EQueueTypeFlagBits::eGraphics, "lighting:unidirectional_shadow:OMNI_SHADOW");
		pass_ci.SetDepthTarget("omni_shadows_target");
	
		pass_ci.AddReadResource("GeometryBatches");
		pass_ci.AddReadResource("IndexedDrawIndirectBuffer");
		pass_ci.AddReadResource("IndirectCountBuffer");
	
		AddRenderPass("omni_shadow_map_pass", pass_ci);
	}
	
	// visibility buffer write
	{
		FRenderConfigRenderCreateInfo pass_ci = makeGraphicsPassCI(ERenderingType::eGeometry, EQueueTypeFlagBits::eGraphics, "visibilitybuffer:write");
		pass_ci.SetDepthTarget("zbuffer_depth_target");
		//pass_ci.SetStencilTarget("zbuffer_depth_target");
	
		pass_ci.AddWriteResource("visbuffer_geometry_target", glm::vec4(0.f, 0.f, 0.f, 0.f));
		pass_ci.AddWriteResource("visbuffer_vboinst_target", glm::vec4(0.f, 0.f, 0.f, 0.f));
		//pass_ci.AddWriteResource("motion_vectors_target");
		pass_ci.AddReadResource("GeometryBatches");
		pass_ci.AddReadResource("IndexedDrawIndirectBuffer");
		pass_ci.AddReadResource("IndirectCountBuffer");
	
		AddRenderPass("visibility_buffer_write_pass", pass_ci);
	}
	
	{
		FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eGeometry, EQueueTypeFlagBits::eGraphics, "visibilitybuffer:shade");
		pass_ci.AddWorkgroupSizeByRT("hdr0", true);
	
		pass_ci.BindWriteResource("write_color", "hdr0");
		pass_ci.BindReadResource("prev_frame_color", "hdr0_history");
		pass_ci.BindReadResource("visbuffer_geometry", "visbuffer_geometry_target");
		pass_ci.BindReadResource("visbuffer_vboinst", "visbuffer_vboinst_target");
		pass_ci.BindReadResource("brdf_lut", "brdf_lut_target");
		pass_ci.BindReadResource("irradiance_cube", "irradiance_cube_target");
		pass_ci.BindReadResource("prefiltered_cube", "prefiltered_cube_target");
		pass_ci.BindReadResource("cascaded_shadowmap", "cascaded_shadows_target");
		pass_ci.BindReadResource("direct_shadowmap", "directional_shadows_target");
		pass_ci.BindReadResource("omni_shadowmap", "omni_shadows_target");
		pass_ci.BindReadResource("depth_buffer", "zbuffer_depth_target");
	
		pass_ci.AddReadResource("GeometryBatches");
		pass_ci.AddReadResource("IndexedDrawIndirectBuffer");
	
		AddRenderPass("compute_materials_pass", pass_ci);
	}
	
	// Compute static motion vectors (can be started in another queue)
	{
		FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, use_low_precision_velocity ? "postprocess:generate_motion_vectors:LOW_PRECISION_VELOCITY" : "postprocess:generate_motion_vectors");
		pass_ci.AddWorkgroupSizeByRT("zbuffer_depth_target", true);
	
		pass_ci.BindWriteResource("write_color", "motion_vectors_target");
		pass_ci.BindReadResource("depth_texture", "zbuffer_depth_target");
	
		AddRenderPass("generate_motion_vectors_pass", pass_ci);
	}
	
	// Anti - alias step
	//{
	//	FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:filmic_smaa_tu2x");
	//	pass_ci.AddWorkgroupSizeByRT("hdr1", true);
	//
	//	pass_ci.BindWriteResource("write_color", "hdr1");
	//	pass_ci.BindWriteResource("current_frame", "hdr0");
	//	pass_ci.BindWriteResource("prev_frame", "hdr0_history");
	//
	//	AddRenderPass("compute_smaa_pass", pass_ci);
	//}
	
	auto input_target = "hdr0";
	auto output_target = "hdr1";
	
	// TemporalAA
	//if (enable_taa)
	//{
	//	FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:compute_temporal_aa");
	//	pass_ci.AddWorkgroupSizeByRT(next_target, true);
	//
	//	pass_ci.BindWriteResource("write_color", next_target);
	//	pass_ci.BindWriteResource("current_color", last_target);
	//	pass_ci.BindWriteResource("prev_color", "hdr0_history");
	//	pass_ci.BindWriteResource("motion_vectors", "motion_vectors_target");
	//	pass_ci.BindWriteResource("depth_image", "zbuffer_depth_target");
	//
	//	AddRenderPass("resolve_temporal_aa_pass", pass_ci);
	//
	//	std::swap(input_target, output_target);
	//}
	
	// Copy history
	{
		FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "utility:copy_compute");
		pass_ci.AddWorkgroupSizeByRT(input_target, true);
	
		pass_ci.BindReadResource("sampler_color", input_target);
		pass_ci.BindWriteResource("write_color", "hdr0_history");
	
		AddRenderPass("copy_final_color_pass", pass_ci);
	}
	
	std::string final_target = input_target;
	
	// Post processing
	{
		u32 pass_count{ 0u };
	
		RENDER_DEBUG_MARKER_ZONE(postprocess, 0xffff33ff);
	
		// FXAA 
		if (enable_fxaa)
		{
			FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:apply_effect:FXAA");
			pass_ci.AddWorkgroupSizeByRT(input_target, true);
	
			pass_ci.BindWriteResource("write_color", output_target);
			pass_ci.BindReadResource("sampler_color", input_target);
	
			AddRenderPass("apply_fxaa", pass_ci);
	
			SWAP_COUNT(input_target, output_target);
		}
	
		// Compute motion blur
		if (motion_blur_enabled)
		{
			RENDER_DEBUG_MARKER_ZONE(compute_motion_blur, 0xffff33ff);
	
			// Apply motion blur
			{
				FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, use_low_precision_velocity ? "postprocess:apply_effect:MOTION_BLUR:LOW_PRECISION_VELOCITY" : "postprocess:apply_effect:MOTION_BLUR");
				pass_ci.AddWorkgroupSizeByRT(input_target, true);
	
				pass_ci.BindReadResource("sampler_color", input_target);
				pass_ci.BindReadResource("motion_vectors", "motion_vectors_target");
				pass_ci.BindWriteResource("write_color", output_target);
	
				AddRenderPass("apply_motion_blur", pass_ci);
			}
	
			SWAP_COUNT(input_target, output_target);
		}
	
		// Compute dof
		if (depth_of_field_enabled)
		{
			RENDER_DEBUG_MARKER_ZONE(compute_depth_of_field, 0xffff33ff);
	
			// Compute bokeh
			{
				FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:bokeh");
				pass_ci.AddWorkgroupSizeByRT(input_target, true);
	
				pass_ci.BindWriteResource("write_color", "hdr0_div2");
				pass_ci.BindReadResource("sampler_color", input_target);
	
				AddRenderPass("compute_bokeh_blur_pass", pass_ci);
			}
	
			// Apply dof
			{
				FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:apply_effect:DOF");
				pass_ci.AddWorkgroupSizeByRT(input_target, true);
	
				pass_ci.BindWriteResource("write_color", output_target);
				pass_ci.BindReadResource("sampler_color", input_target);
				pass_ci.BindReadResource("depth_image", "zbuffer_depth_target");
				pass_ci.BindReadResource("bokeh_color", "hdr0_div2");
	
				AddRenderPass("apply_bokeh_blur_pass", pass_ci);
			}
	
			SWAP_COUNT(input_target, output_target);
		}
	
		// Compute bloom
		{
			RENDER_DEBUG_MARKER_ZONE(compute_bloom, 0xffff33ff);
	
			// Starting from brightness detection pass
			{
				FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, bloom_use_brightness ? "postprocess:brightdetect" : "utility:copy_compute");
				pass_ci.AddWorkgroupSizeByRT("hdr0_div2", true);
	
				pass_ci.BindReadResource("sampler_color", input_target);
				pass_ci.BindWriteResource("write_color", "hdr0_div2");
	
				AddRenderPass("brightdetect_pass", pass_ci);
			}
	
			// Begin downsampling
			{
				FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:downsample:FIRST_PASS");
				
				pass_ci.BindReadResource("sampler_color", "hdr0_div2");
	
				RENDER_DEBUG_MARKER_ZONE(downsample, 0xffff33ff);
				for (u32 idx = 0; idx < bloom_blur_depth; ++idx)
				{
					pass_ci.AddWorkgroupSizeByRT("bloom_target", idx, true);
					pass_ci.BindWriteResource("write_color", "bloom_target", idx);
	
					AddRenderPass(std::format("downsample_pass_{}", idx), pass_ci);
	
					pass_ci.ClearResource();
					pass_ci.BindReadResource("sampler_color", "bloom_target", idx);
	
					if(idx == 0u)
						pass_ci.shader = "postprocess:downsample";
				}
			}
	
			// And upsampling
			{
				RENDER_DEBUG_MARKER_ZONE(upsample, 0xffff33ff);
				for (u32 idx = bloom_blur_depth - 1; idx > 0; --idx)
				{
					FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:upsample");
					pass_ci.AddWorkgroupSizeByRT("bloom_target", idx - 1, true);
	
					pass_ci.BindReadResource("sampler_color", "bloom_target", idx);
					pass_ci.BindWriteResource("write_color", "bloom_target", idx - 1);
	
					AddRenderPass(std::format("upnsample_pass_{}", idx), pass_ci);
				}
			}
	
			// Apply bloom
			{
				FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:apply_effect:BLOOM");
				pass_ci.AddWorkgroupSizeByRT(output_target, true);
	
				pass_ci.BindWriteResource("write_color", output_target);
				pass_ci.BindReadResource("sampler_color", input_target);
				pass_ci.BindReadResource("bloom_color", "bloom_target", 0);
				pass_ci.BindReadResource("dirt_mask", "LensDirtMask");
	
				AddRenderPass("apply_bloom", pass_ci);
			}
	
			SWAP_COUNT(input_target, output_target);
		}
	
		// Compute chromatic aberration
		if (chromatic_aberration_enabled)
		{
			FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:apply_effect:CHROMATIC_ABERRATION");
			pass_ci.AddWorkgroupSizeByRT(input_target, true);
	
			pass_ci.BindWriteResource("write_color", output_target);
			pass_ci.BindReadResource("sampler_color", input_target);
	
			AddRenderPass("apply_chromatic_aberration_pass", pass_ci);
		
			SWAP_COUNT(input_target, output_target);
		}
	
		// Tonemap pass
		{
			RENDER_DEBUG_MARKER_ZONE(compute_tonemap, 0xffff33ff);
	
			// Compute histogram
			{
				FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:compute_histogram");
				pass_ci.AddWorkgroupSizeByRT(input_target, true);
	
				pass_ci.BindReadResource("sampler_color", input_target);
				pass_ci.BindWriteResource("Histogram", "histogram_buffer");
	
				AddRenderPass("compute_histogram_pass", pass_ci);
			}
	
			// Compute avg screen space brightness
			{
				FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:compute_brightness");
				pass_ci.AddWorkgroupSizeManual(1, 1, 1, false);
	
				pass_ci .BindWriteResource("target", "avg_brightness_target");
				pass_ci .BindReadResource("Histogram", "histogram_buffer");
	
				AddRenderPass("compute_brightness_pass", pass_ci);
			}
	
			// Apply tonemapping
			{
				FRenderConfigComputeCreateInfo pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:apply_effect:TONEMAP");
				pass_ci.AddWorkgroupSizeByRT(input_target, true);
	
				pass_ci.BindWriteResource("write_color", output_target);
				pass_ci.BindReadResource("sampler_color", input_target);
				pass_ci.BindReadResource("avg_luminosity", "avg_brightness_target");
	
				AddRenderPass("apply_tonemap_pass", pass_ci);
			}
	
			SWAP_COUNT(input_target, output_target);
		}
	
		final_target = pass_count % 2 ? input_target : output_target;
	}
	
	// Present pass
	{
		FRenderConfigRenderCreateInfo pass_ci = makeGraphicsPassCI(ERenderingType::eFullscreen, EQueueTypeFlagBits::eGraphics, "testing:screenspace");
		pass_ci.FlipFiewport();
	
		pass_ci.BindReadResource("input_image", final_target);
		pass_ci.AddWriteResource("present_target");
	
		AddRenderPass("finalize_image_pass", pass_ci);
	}

	// ImGui pass
	{
		FRenderConfigRenderCreateInfo pass_ci = makeGraphicsPassCI(ERenderingType::eImGui, EQueueTypeFlagBits::eGraphics, "testing:imgui");
		pass_ci.AddWriteResource("present_target");
		AddRenderPass("imgui_pass", pass_ci);
	}
}

FAPIHandle CRenderGraph::AddStaticResource(const std::string& name, const std::string& path)
{
	log_debug("[Render Graph] Loading static resource: {}, path: {}.", name, path);

	auto& render_context = m_pRenderInterface->GetRenderContext();

	m_nameToResource[name] = FGraphHandle(m_vRenderResources.size());

	auto& new_render_resource = m_vRenderResources.emplace_back();
	new_render_resource.type = ERenderResourceTypeFlagBits::eStaticImage;

	FTextureCreateInfo create_info{}; 
	FTextureUploadInfo upload_info{};
	new_render_resource.handle = m_pRenderInterface->LoadTexture(path, create_info, upload_info);

	if (!new_render_resource.handle)
		new_render_resource.handle = render_context->CreateTexture(name, create_info, std::move(upload_info));

	return new_render_resource.handle;
}

FAPIHandle CRenderGraph::AddRenderResource(const std::string& name, const FTextureCreateInfo& texture_ci)
{
	log_debug("[Render Graph] Allocating render target: {}, size({}x{}x{}) * {}, mips: {}, layers: {}, format: {}.", name, 
		texture_ci.size.width, texture_ci.size.height, texture_ci.size.depth, texture_ci.size.scale,
		texture_ci.mipLevels, texture_ci.arrayLayers, utl::cast<u32>(texture_ci.format));

	auto& render_context = m_pRenderInterface->GetRenderContext();
	auto render_extent = render_context->GetRenderResolution();
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
	new_render_resource.handle = render_context->CreateTexture(name, texture_create_info);
	new_render_resource.type = (texture_ci.flags & ETextureCreateFlagBits::eStorage) ? ERenderResourceTypeFlagBits::eStorageTarget : ERenderResourceTypeFlagBits::eRenderTarget;
	
	m_nameToResource[name] = FGraphHandle(resource_index);
	return new_render_resource.handle;
}

FAPIHandle CRenderGraph::AddRenderResource(const std::string& name, const FBufferCreateInfo& buffer_ci)
{
	log_debug("[Render Graph] Allocating buffer: {}, size: {}b.", name,
		buffer_ci.size * buffer_ci.count);

	auto& render_context = m_pRenderInterface->GetRenderContext();

	m_nameToResource[name] = FGraphHandle(m_vRenderResources.size());

	auto& new_render_resource = m_vRenderResources.emplace_back();
	new_render_resource.handle = render_context->CreateBuffer(name, buffer_ci);
	new_render_resource.type = ERenderResourceTypeFlagBits::eBuffer;

	return new_render_resource.handle;
}

void CRenderGraph::SetBackbufferResourceName(const std::string& name)
{
	if (m_backBufferResource)
	{
		log_warning("[Render Graph] Backbuffer name is already set. Adding new name association: {}.", name);
		m_nameToResource[name] = m_backBufferResource;
		return;
	}

	auto resource_index = m_vRenderResources.size();

	auto& new_render_resource = m_vRenderResources.emplace_back();
	new_render_resource.type = ERenderResourceTypeFlagBits::eBackbuffer;

	m_backBufferResource = FGraphHandle(resource_index);
	m_nameToResource[name] = m_backBufferResource;
}

void CRenderGraph::PushDebugMarker(const std::string& name, u32 color, EQueueTypeFlagBits queue)
{
	auto new_marker = std::make_unique<CBeginDebugMarker>(m_pRenderInterface);
	new_marker->Init(name, color, queue);

	auto marker_index = m_vRenderPasses.size();
	m_vPassExecutionQueue.emplace_back(marker_index);

	m_vRenderPasses.emplace_back(std::move(new_marker));
	m_nameToPass[name + "_begin"] = marker_index;
}

void CRenderGraph::PopDebugMarker(const std::string& name, EQueueTypeFlagBits queue)
{
	auto new_marker = std::make_unique<CEndDebugMarker>(m_pRenderInterface);
	new_marker->Init(queue);

	auto marker_index = m_vRenderPasses.size();
	m_vPassExecutionQueue.emplace_back(marker_index);

	m_vRenderPasses.emplace_back(std::move(new_marker));
	m_nameToPass[name + "_end"] = marker_index;
}

const FRenderResource& CRenderGraph::GetRenderResource(const std::string& name) const
{
	static FRenderResource empty_resource{};
	
	auto found = m_nameToResource.find(name);
	if (found != m_nameToResource.end())
		return m_vRenderResources[found->second.Get()];

	return empty_resource;
}

const FRenderResource& CRenderGraph::GetRenderResource(FGraphHandle resource_id) const
{
	return m_vRenderResources[resource_id.Get()];
}

FGraphHandle CRenderGraph::GetRenderRersourceId(const std::string& name) const
{
	auto found = m_nameToResource.find(name);
	if (found != m_nameToResource.end())
		return found->second;

	return FGraphHandle(INVALID_HANDLE);
}

u64 CRenderGraph::AddRenderPass(const std::string& name, const FRenderConfigRenderCreateInfo& render_pass_ci)
{
	log_info("[Render Graph] Add render pass: {}", name);

	u64 pass_index = m_vRenderPasses.size();
	m_vPassExecutionQueue.emplace_back(pass_index);

	FRenderPassCreateInfo pass_ci{};
	pass_ci.flipViewport = render_pass_ci.flipViewportState;

	ApplyRenderPassResources(pass_index, &render_pass_ci, &pass_ci);

	std::unique_ptr<IRenderPass> new_pass{};
	if (render_pass_ci.type == ERenderingType::eFullscreen)
		new_pass = std::make_unique<CFullscreenPass>(m_pRenderInterface);
	else if (render_pass_ci.type == ERenderingType::eGeometry)
		new_pass = std::make_unique<CGeometryPass>(m_pRenderInterface);
	else if (render_pass_ci.type == ERenderingType::eCascadedShadowMap)
		new_pass = std::make_unique<CCascadedShadowMapPass>(m_pRenderInterface);
	else if (render_pass_ci.type == ERenderingType::eDirectionalShadowMap)
		new_pass = std::make_unique<CDirectionalShadowMapPass>(m_pRenderInterface);
	else if (render_pass_ci.type == ERenderingType::eOmniShadowMap)
		new_pass = std::make_unique<COmnilShadowMapPass>(m_pRenderInterface);
	else if(render_pass_ci.type == ERenderingType::eClear)
		new_pass = std::make_unique<CClearPass>(m_pRenderInterface);
	else if (render_pass_ci.type == ERenderingType::eImGui)
		new_pass = std::make_unique<CImGuiPass>(m_pRenderInterface);

	if (!render_pass_ci.depthResource.empty())
		pass_ci.depthTarget = GetRenderRersourceId(render_pass_ci.depthResource);

	if (!render_pass_ci.stencilResource.empty())
		pass_ci.depthTarget = GetRenderRersourceId(render_pass_ci.stencilResource);

	new_pass->Init(pass_ci);
	m_vRenderPasses.emplace_back(std::move(new_pass));
	m_nameToPass[name] = pass_index;

	return pass_index;
}

u64 CRenderGraph::AddRenderPass(const std::string& name, const FRenderConfigComputeCreateInfo& compute_pass_ci)
{
	log_info("[Render Graph] Add compute pass: {}", name);

	u64 pass_index = m_vRenderPasses.size();
	m_vPassExecutionQueue.emplace_back(pass_index);

	FComputePassCreateInfo pass_ci{};
	ApplyRenderPassResources(pass_index, &compute_pass_ci, &pass_ci);

	std::unique_ptr<IComputePass> new_pass{};
	if (compute_pass_ci.type == EComputeType::eDefault)
		new_pass = std::make_unique<CComputePass>(m_pRenderInterface);
	else if (compute_pass_ci.type == EComputeType::eGeometry)
		new_pass = std::make_unique<CComputeGeometryPass>(m_pRenderInterface);
	else if (compute_pass_ci.type == EComputeType::eCubemap)
		new_pass = std::make_unique<CComputeCubemapPass>(m_pRenderInterface);
	else if (compute_pass_ci.type == EComputeType::eCulling)
		new_pass = std::make_unique<CComputeCullingPass>(m_pRenderInterface);
	else if (compute_pass_ci.type == EComputeType::ePostProcess)
		new_pass = std::make_unique<CPostProcessPass>(m_pRenderInterface);

	for (const auto& group_info : compute_pass_ci.groupInfos)
	{
		auto& new_group_info = pass_ci.groupInfos.emplace_back();
		new_group_info.type = group_info.type;
		new_group_info.divideByLocals = group_info.divideByLocals;
		new_group_info.target = GetRenderRersourceId(group_info.target);
		new_group_info.width = group_info.width;
		new_group_info.height = group_info.height;
		new_group_info.depth = group_info.depth;
		new_group_info.subresource = group_info.subresource;
	}

	new_pass->Init(pass_ci);
	m_vRenderPasses.emplace_back(std::move(new_pass));
	m_nameToPass[name] = pass_index;

	return pass_index;
}

template <class _Ty, class _KTy>
struct counter_pair : std::pair <_Ty, _KTy>
{
	using std::pair<_Ty, _KTy>::pair;

	bool operator== (const counter_pair& other)
	{
		std::pair p1 = *this;
		std::pair p2 = other;

		return p1 == p2;
	}
};

void CRenderGraph::Build()
{
	if (!m_backBufferResource)
	{
		log_error("[Render Graph] Failed to Build render graph. Reason: back buffer resource is not set.");
		return;
	}

	auto& backbuffer_resource = m_vRenderResources[m_backBufferResource.Get()];
	if (!backbuffer_resource.usages.count(ERenderResourceAccessFlagBits::eWrite))
	{
		log_error("[Render Graph] Failed to Build render graph. Reason: No one writes to back buffer resource.");
		return;
	}


	utl::hash_map<u64, FPassBuildInfo> build_infos;
	GenerateResourceBarriers(build_infos, true);

	// Barriers post process

	//m_vPassDependencies.resize(m_vRenderPasses.size());
	//m_vPassMergeDependencies.resize(m_vRenderPasses.size());
	//m_vPassProcessing.resize(m_vRenderPasses.size());
	//
	//for (u64 pass_index : backbuffer_resource.usages[ERenderResourceAccessFlagBits::eWrite])
	//	TraverseDependencies(pass_index);
	//
	//std::reverse(m_vPassExecutionQueue.begin(), m_vPassExecutionQueue.end());
	//filter_pass_queue(m_vPassExecutionQueue);
	//
	//ReorderPasses(m_vPassExecutionQueue);
	//

	// Build render passes
	for (u64 pass_index : m_vPassExecutionQueue)
	{
		auto& build_info = build_infos[pass_index];
		auto& pass = m_vRenderPasses.at(pass_index);
		pass->Build(build_info);
	}

	return;
}

void CRenderGraph::PostLoad()
{
	for (u64 pass_index : m_vPassExecutionQueue)
	{
		auto& pass = m_vRenderPasses.at(pass_index);
		pass->PostLoad();
	}
}

void CRenderGraph::Update(f32 dt)
{

}

void CRenderGraph::Execute(f32 dt)
{
	auto& render_context = m_pRenderInterface->GetRenderContext();

	for (u64 pass_index : m_vPassExecutionQueue)
	{
		auto& pass = m_vRenderPasses[pass_index];
		auto& resource_barriers = m_perPassResourceBarriers[pass_index];

		if (!pass->CanExecute())
			continue;

		if (pass->m_debugName == "fullscreen:testing:screenspace")
			int iiiii = 0;

		if (pass->m_debugName == "imgui:testing:imgui")
			int iiiii = 0;

		render_context->ProcessResourceBarriers(pass->m_queue, resource_barriers);
		pass->Execute(dt);
	}

	auto& backbuffer_resource = m_vRenderResources[m_backBufferResource.Get()];
	utl::vector<FResourceBarrierInfo> backbuffer_barriers{};
	
	auto& backbuffer_barrier = backbuffer_barriers.emplace_back();
	backbuffer_barrier.resource.handle = backbuffer_resource.handle;
	backbuffer_barrier.resource.type = backbuffer_resource.type;
	backbuffer_barrier.dependencyType = packDependencyBarrier(ERenderResourceAccessFlagBits::eWrite, ERenderResourceAccessFlagBits::eRead);
	backbuffer_barrier.srcPassType = EPassType::eRender;
	backbuffer_barrier.dstPassType = EPassType::eNone;
	backbuffer_barrier.barrierType = ERenderResourceBarrierType::eFinal;
	
	render_context->ProcessResourceBarriers(EQueueTypeFlagBits::eGraphics, backbuffer_barriers);
}

void CRenderGraph::ApplyRenderPassResources(u64 pass_index, const FRenderConfigPassCreateInfo* raw_pass_ci, FPassCreateInfo* pass_ci)
{
	pass_ci->queue = raw_pass_ci->queue;
	pass_ci->shader = raw_pass_ci->shader;
	pass_ci->oneTime = raw_pass_ci->oneTime;

	// Binded resources for using as input
	for (const auto& [access_flag, bindings] : raw_pass_ci->resourceBindings)
	{
		for (const auto& [binding_point, resource_binding] : bindings)
		{
			auto resource_id = GetRenderRersourceId(resource_binding.name);
			auto& render_resource = GetRenderResource(resource_id);

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
				pass_ci->resources[resource_key].emplace_back(FRenderResourceBinding{ resource_id, resource_binding.subresource });
			}

			pass_ci->bindings.emplace(binding_point, FRenderResourceBinding{ resource_id, resource_binding.subresource });
		}
	}

	for (const auto& [access_flag, resources] : raw_pass_ci->otherResources)
	{
		for (const auto& [resource_name, clear_values] : resources)
		{
			auto resource_id = GetRenderRersourceId(resource_name);
			auto& render_resource = GetRenderResource(resource_id);

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

			// TODO: Don't know is subresource needed for writing
			auto resource_key = packResourceGroup(access_flag, group_type);	
			pass_ci->resources[resource_key].emplace_back(FRenderResourceBinding{ resource_id, ~0u });
			pass_ci->clearValues.emplace(render_resource.handle, clear_values);
		}
	}

	ApplyMultiRenderResourceDependencies(pass_index, pass_ci->queue, pass_ci->resources);
}

void CRenderGraph::AapplySingleRenderResourceDependencies(FGraphHandle resource_id, ERenderResourceAccessFlagBits access_type, u64 pass_index, EQueueTypeFlagBits queue_type)
{
	auto& render_resource = m_vRenderResources[resource_id.Get()];
	render_resource.usages[access_type].emplace(pass_index);
	render_resource.queuesUsed |= queue_type;
}

void CRenderGraph::ApplyMultiRenderResourceDependencies(u64 pass_index, EQueueTypeFlagBits queue_type, const utl::hash_map<ERenderResourceGroupType, utl::vector<FRenderResourceBinding>>& associated_resources)
{
	for (const auto& [Type, resource_bindings] : associated_resources)
	{
		auto access_type = utl::cast<ERenderResourceAccessFlagBits>(utl::enum_cast(Type) & 0xFFFF);
		for (const auto& resource_binding : resource_bindings)
			AapplySingleRenderResourceDependencies(resource_binding.resource, access_type, pass_index, queue_type);
	}
}

bool CRenderGraph::WasGraphChanged()
{
	static std::hash<u64> hasher;
	u64 new_hash{ 0ull };
	for (u64 pass_index : m_vPassExecutionQueue)
	{
		auto& pass = m_vRenderPasses[pass_index];

		if (pass->CanExecute())
			new_hash ^= hasher(pass_index);
	}

	bool res{ new_hash != m_executionHash };
	m_executionHash = new_hash;
	return res;
}

void CRenderGraph::GenerateResourceBarriers(utl::hash_map<u64, FPassBuildInfo>& build_infos, bool generate_build_infos)
{
	utl::hash_map<std::pair<FGraphHandle, u32>, tmp_access_info, pair_hash> resource_access_map{};
	utl::hash_map<FGraphHandle, std::unordered_set<u64>> per_res_accessed_passes{};

	for (u64 pass_index : m_vPassExecutionQueue)
	{
		auto& pass = m_vRenderPasses.at(pass_index);

		for (const auto& [resource_group, resource_bindings] : pass->m_renderResources)
		{
			ERenderResourceAccessFlagBits access_flag{};
			ERenderResourceGroupFlagBits group_flag{};
			unpackResourceGroup(resource_group, access_flag, group_flag);

			for (u64 resource_index = 0ull; resource_index < resource_bindings.size(); ++resource_index)
			{
				const auto& resource_binding = resource_bindings[resource_index];
				const auto& render_resource = m_vRenderResources[resource_binding.resource.Get()];

				bool is_backbuffer_resource = m_backBufferResource == resource_binding.resource;

				// Save all accessed passes for check is this last access
				auto& accessed_passes = per_res_accessed_passes[resource_binding.resource];
				accessed_passes.emplace(pass_index);

				// Barriers is not needed for static resource (Loaded from file). 
				if (render_resource.type & ERenderResourceTypeFlagBits::eStaticImage)
					continue;

				// Check will this resource even be writen by any pass
				if (is_x_only_access(render_resource, ERenderResourceAccessFlagBits::eRead))
					continue;

				u32 subresource_key = resource_binding.subresource == ~0ull ? 0u : resource_binding.subresource;

				auto resource_key = std::make_pair(resource_binding.resource, subresource_key);
				auto& last_resource_access = resource_access_map[resource_key];

				// Ignoring read after read barriers
				if (last_resource_access.access == access_flag && access_flag == ERenderResourceAccessFlagBits::eRead)
					continue;

				auto barrier_type = packDependencyBarrier(last_resource_access.access, access_flag);

				// Check for last access duplicating
				if (last_resource_access.last_barrier)
				{
					auto& last_barrier = *last_resource_access.last_barrier;
					if (
						last_barrier.dependencyType == barrier_type &&
						last_barrier.resource.handle == render_resource.handle &&
						last_barrier.resource.type == render_resource.type &&
						last_barrier.subresourceIndex == resource_binding.subresource &&
						last_barrier.srcPassType == last_resource_access.pass_type &&
						last_barrier.dstPassType == pass->Type()
						)
						continue;
				}

				// Prepare infos for render pipeline
				auto& build_info = build_infos[pass_index];
				auto& attachment = build_info.attachments[resource_binding.resource];
				attachment.loadOp = barrier_type == ERenderResourceDependencyBarrierType::eWriteAfterWrite ? ERenderResourceLoadOp::eLoad : ERenderResourceLoadOp::eClear;
				attachment.storeOp = (render_resource.usages.count(ERenderResourceAccessFlagBits::eRead) || resource_binding.resource == m_backBufferResource) ? ERenderResourceStoreOp::eStore : ERenderResourceStoreOp::eDontCare;

				// TODO: Ignore duplicate barriers
				auto& pass_barriers = m_perPassResourceBarriers[pass_index];
				auto& new_barrier = pass_barriers.emplace_back();

				// First time barrier should be translation barrier and last barrier should be if resource is read only
				// TODO: Add special enum class for it
				bool is_change_pipeline_ownership = last_resource_access.pass_type == pass->Type() && barrier_type == ERenderResourceDependencyBarrierType::eWriteAfterWrite;

				if (last_resource_access.access == ERenderResourceAccessFlagBits::eNone)
					new_barrier.barrierType = access_flag == ERenderResourceAccessFlagBits::eRead ? new_barrier.barrierType = ERenderResourceBarrierType::eFinal : ERenderResourceBarrierType::eIntermediate;
				else if (is_ro_access_only_remain(render_resource, pass_index))
					new_barrier.barrierType = is_backbuffer_resource ? ERenderResourceBarrierType::eIntermediate : ERenderResourceBarrierType::eFinal;
				else if (is_change_pipeline_ownership)
					new_barrier.barrierType = ERenderResourceBarrierType::eIntermediate;

				new_barrier.dependencyType = barrier_type;
				new_barrier.resource.handle = render_resource.handle;
				new_barrier.resource.type = render_resource.type;
				new_barrier.subresourceIndex = resource_binding.subresource;
				new_barrier.srcPassType = last_resource_access.pass_type;
				new_barrier.dstPassType = pass->Type();

				last_resource_access.access = access_flag;
				last_resource_access.all_time_access.emplace_back(access_flag);
				last_resource_access.pass_type = new_barrier.dstPassType;
				last_resource_access.last_barrier = &new_barrier;
			}
		}
	}
}

void CRenderGraph::TraverseDependencies(u64 current_pass_index, u64 stack_count)
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

		for (const auto& resource_access : resource_indices)
		{
			auto& render_resource = m_vRenderResources[resource_access.resource.Get()];
			if (!render_resource.usages.count(ERenderResourceAccessFlagBits::eWrite))
				continue;

			auto& writer_passes = render_resource.usages[ERenderResourceAccessFlagBits::eWrite];

			if (stack_count > m_vRenderPasses.size())
				log_error("[Render Graph] Cycle detected.");

			for (u64 write_pass_index : writer_passes)
			{
				if (write_pass_index == current_pass_index)
					continue;

				m_vPassDependencies.at(current_pass_index).emplace(write_pass_index);

				if (group_flag == ERenderResourceGroupFlagBits::eColor)
					m_vPassMergeDependencies.at(current_pass_index).emplace(write_pass_index);

				stack_count++;
				TraverseDependencies(write_pass_index, stack_count);
				stack_count--;
			}
		}
	}
}

bool CRenderGraph::CheckPassDependencies(u64 dst_pass, u64 src_pass)
{
	if (dst_pass == src_pass)
		return true;

	for (const auto& dep : m_vPassDependencies[dst_pass])
	{
		if (CheckPassDependencies(dep, src_pass))
			return true;
	}

	return false;
}

void CRenderGraph::ReorderPasses(utl::vector<u64>& flattened_passes)
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
				if (CheckPassDependencies(dependee, merge_dep))
					continue;

				if (merge_dep != dependee)
					m_vPassDependencies[merge_dep].insert(dependee);
			}
		}
	}

	if (flattened_passes.size() <= 2ull)
		return;

	utl::vector<u64> unscheduled_passes;
	unscheduled_passes.reserve(m_vRenderPasses.size());
	std::swap(flattened_passes, unscheduled_passes);

	const auto schedule = [&](unsigned index) 
		{
		// Need to preserve the order of remaining elements.
		flattened_passes.push_back(unscheduled_passes[index]);
		std::move(unscheduled_passes.begin() + index + 1,
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
				overlap_factor = INVALID_HANDLE;
			else
			{
				for (auto itr = flattened_passes.rbegin(); itr != flattened_passes.rend(); ++itr)
				{
					if (CheckPassDependencies(unscheduled_passes[i], *itr))
						break;
					overlap_factor++;
				}
			}

			if (overlap_factor <= best_overlap_factor)
				continue;

			bool possible_candidate = true;
			for (u64 j = 0; j < i; j++)
			{
				if (CheckPassDependencies(unscheduled_passes[i], unscheduled_passes[j]))
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