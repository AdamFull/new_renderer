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
#include "render_passes/DebugDrawPass.h"
#include "render_passes/ImGuiPass.h"

#include <engine/VariableBlock.h>
#include <engine/Variables.h>

using namespace engine;
using namespace engine::render;

inline u64 aligned_size(u64 size, u64 alignment)
{
	return (size + alignment - 1) & ~(alignment - 1);
}

template<class _HeaderType>
inline void allocate_combined_space(u32 type, void** out, u64 size, u64 alignment)
{
	u64 aligned_header_size = aligned_size(sizeof(_HeaderType), alignment);
	u64 aligned_data_size = aligned_size(size, alignment);
	(*out) = _aligned_malloc(aligned_header_size + aligned_data_size, alignment);

	_HeaderType* header = reinterpret_cast<_HeaderType*>(*out);
	header->type = type;
	header->data_offset = aligned_header_size;

	std::memset(reinterpret_cast<char*>(header) + header->data_offset, 0, aligned_data_size);
}

inline void deallocate_graph_node(FRenderGraphNodeHeader** out)
{
	if ((*out))
	{
		_aligned_free((*out));
		(*out) = nullptr;

	}
}

template<class _Ty>
FRenderGraphNodeHeader* allocate_graph_node(ERenderGraphNodeType type)
{
	FRenderGraphNodeHeader* out{ nullptr };
	allocate_combined_space<FRenderGraphNodeHeader>(static_cast<u32>(type), (void**)&out, sizeof(_Ty), alignof(max_align_t));
	return out;
}

template<class _Ty>
_Ty* cast_graph_node(FRenderGraphNodeHeader* header)
{
	auto* data = reinterpret_cast<char*>(header) + header->data_offset;
	return reinterpret_cast<_Ty*>(data);
}

#define SWAP_COUNT(in, out) std::swap(in, out); ++pass_count;

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

bool was_accessed(const FConsistentAccessInfo& access_info, ERenderResourceAccessFlagBits access_flag)
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

CRenderGraph::~CRenderGraph()
{
	DestoyRenderGraph();
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

void CRenderGraph::EvaluateRenderGraph(ERenderGraphEvaluateFlags flags)
{
	if (flags & ERenderGraphEvaluateFlagBits::eUpdateGraph)
	{
		m_graph.perResourceAccessedPasses.clear();
		m_graph.resourceAccessMap.clear();
		m_graph.passes.clear();
		m_graph.buildInfos.clear();
		m_graph.perPassBarriers.clear();
	}

	for (auto& node : m_vGraphNodes)
		EvaluateRenderGraphNode(flags, node);
}

void CRenderGraph::EvaluateRenderGraphNode(ERenderGraphEvaluateFlags flags, FRenderGraphNodeHeader* header)
{
	if (header)
	{
		switch (static_cast<ERenderGraphNodeType>(header->type))
		{
		case ERenderGraphNodeType::eStaticCondition:
		{
			auto* static_condition_node = cast_graph_node<FRenderGraphStaticConditionNode>(header);
			EvaluateRenderGraphNode(flags, static_condition_node->node);
		} break;
		case ERenderGraphNodeType::eDynamicCondition:
		{
			auto* dynamic_condition_node = cast_graph_node<FRenderGraphDynamicConditionNode>(header);
			if (dynamic_condition_node->condition())
				EvaluateRenderGraphNode(flags, dynamic_condition_node->pass);
			else
				EvaluateRenderGraphNode(flags, dynamic_condition_node->fail);

		} break;
		case ERenderGraphNodeType::eGroupNode:
		{
			auto* group_node = cast_graph_node<FRenderGraphGroupNode>(header);

			auto& render_context = m_pRenderInterface->GetRenderContext();
			if (flags & ERenderGraphEvaluateFlagBits::eRender)
				render_context->BeginDebugLabel(group_node->name.c_str(), group_node->queue, group_node->color);

			for (auto& node : group_node->nodes)
				EvaluateRenderGraphNode(flags, node);

			if (flags & ERenderGraphEvaluateFlagBits::eRender)
				render_context->EndDebugLabel(group_node->queue);

		} break;
		case ERenderGraphNodeType::ePassNode:
		{
			auto* pass_node = cast_graph_node<FRenderGraphPassNode>(header);

			auto& pass = m_vRenderPasses[pass_node->passIndex];
			if (!pass->CanExecute())
				return;

			if (flags & ERenderGraphEvaluateFlagBits::eUpdateGraph)
				GraphUpdateBarriers(pass_node->passIndex);

			if (flags & ERenderGraphEvaluateFlagBits::eRender)
			{
				auto& render_context = m_pRenderInterface->GetRenderContext();
				auto& pass_barriers = m_graph.perPassBarriers[pass_node->passIndex];
				render_context->ProcessResourceBarriers(pass->m_queue, pass_barriers);
				pass->Execute();
			}
		} break;
		}
	}
}
void CRenderGraph::DestoyRenderGraph()
{
	for (auto& node : m_vGraphNodes)
		DestoyRenderGraphNode(node);
}

void CRenderGraph::DestoyRenderGraphNode(FRenderGraphNodeHeader* header)
{
	if (header)
	{
		switch (static_cast<ERenderGraphNodeType>(header->type))
		{
		case ERenderGraphNodeType::eStaticCondition:
		{
			auto* static_condition_node = cast_graph_node<FRenderGraphStaticConditionNode>(header);
			DestoyRenderGraphNode(static_condition_node->node);
			deallocate_graph_node(&static_condition_node->node);
		} break;
		case ERenderGraphNodeType::eDynamicCondition:
		{
			auto* dynamic_condition_node = cast_graph_node<FRenderGraphDynamicConditionNode>(header);

			DestoyRenderGraphNode(dynamic_condition_node->pass);
			deallocate_graph_node(&dynamic_condition_node->pass);

			DestoyRenderGraphNode(dynamic_condition_node->fail);
			deallocate_graph_node(&dynamic_condition_node->fail);

		} break;
		case ERenderGraphNodeType::eGroupNode:
		{
			auto* group_node = cast_graph_node<FRenderGraphGroupNode>(header);

			auto& render_context = m_pRenderInterface->GetRenderContext();

			for (auto& node : group_node->nodes)
			{
				DestoyRenderGraphNode(node);
				deallocate_graph_node(&node);
			}

		} break;
		}
	}
}

void CRenderGraph::GraphUpdateBarriers(u64 pass_index)
{
	auto& pass = m_vRenderPasses[pass_index];
	auto& pass_barriers = m_graph.perPassBarriers[pass_index];

	m_graph.passes.emplace_back(pass_index);

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
			auto& accessed_passes = m_graph.perResourceAccessedPasses[resource_binding.resource];
			accessed_passes.emplace(pass_index);

			// Barriers is not needed for static resource (Loaded from file). 
			if (render_resource.type & ERenderResourceTypeFlagBits::eStaticImage)
				continue;

			// Check will this resource even be writen by any pass
			if (is_x_only_access(render_resource, ERenderResourceAccessFlagBits::eRead))
				continue;

			u32 subresource_key = resource_binding.subresource == ~0ull ? 0u : resource_binding.subresource;

			auto resource_key = std::make_pair(resource_binding.resource, subresource_key);
			auto& last_resource_access = m_graph.resourceAccessMap[resource_key];

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
			auto& build_info = m_graph.buildInfos[pass_index];
			auto& attachment = build_info.attachments[resource_binding.resource];
			attachment.loadOp = barrier_type == ERenderResourceDependencyBarrierType::eWriteAfterWrite ? ERenderResourceLoadOp::eLoad : ERenderResourceLoadOp::eClear;
			attachment.storeOp = (render_resource.usages.count(ERenderResourceAccessFlagBits::eRead) || resource_binding.resource == m_backBufferResource) ? ERenderResourceStoreOp::eStore : ERenderResourceStoreOp::eDontCare;

			// TODO: Ignore duplicate barriers
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
	
	bool tonemap_enabled{ true };
	bool enable_fxaa{ false };
	bool enable_taa{ false };
	bool motion_blur_enabled{ false };
	bool depth_of_field_enabled{ false };
	bool chromatic_aberration_enabled{ false };
	
	//Bloom settings
	bool bloom_enabled{ false };
	bool bloom_use_brightness{ true };
	int bloom_blur_depth{ 7 };
	
	// Loading test cubemap
	{
		m_pRenderInterface->CreateTextureObject("/root/embed/rendering/textures/CasualDay4K.ktx2");
		m_pRenderInterface->CreateTextureObject("/root/embed/rendering/textures/DirtMaskTextureExample.png");
	}
	
	// GI targets
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eRG16_SFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		texture_ci.SetFixedSize2D(1024);
		m_pRenderInterface->CreateTextureObject("brdf_lut_target", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTextureCubeCI(EApiDataFormat::eRGBA32_SFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		texture_ci.SetFixedSize2D(64);
		m_pRenderInterface->CreateTextureObject("irradiance_cube_target", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTextureCubeCI(EApiDataFormat::eRGBA16_SFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		texture_ci.SetFixedSize2D(512);
		texture_ci.EnableMipMap();
		m_pRenderInterface->CreateTextureObject("prefiltered_cube_target", texture_ci);
	}
	
	// Shadow targets
	{
		FTextureCreateInfo texture_ci = MakeTexture2DArrayCI(EApiDataFormat::eD32SFLOAT, ETextureCreateFlagBits::eSamplable);
		texture_ci.SetFixedSize2D(cascade_shadow_map_resolution);
		texture_ci.arrayLayers = cascade_shadow_map_cascade_count * max_directional_light_shadow_count;
		m_pRenderInterface->CreateTextureObject("cascaded_shadows_target", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTexture2DArrayCI(EApiDataFormat::eD16UNORM, ETextureCreateFlagBits::eSamplable);
		texture_ci.SetFixedSize2D(spot_light_shadow_map_resolution);
		texture_ci.arrayLayers = max_spot_light_shadow_count;
		m_pRenderInterface->CreateTextureObject("directional_shadows_target", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTextureCubeArrayCI(EApiDataFormat::eD16UNORM, ETextureCreateFlagBits::eSamplable);
		texture_ci.SetFixedSize2D(point_light_shadow_map_resolution);
		texture_ci.arrayLayers = max_point_light_shadow_count;
		m_pRenderInterface->CreateTextureObject("omni_shadows_target", texture_ci);
	}
	
	// Visibility buffer targets
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eRGBA8_UNORM, ETextureCreateFlagBits::eSamplable);
		m_pRenderInterface->CreateTextureObject("visbuffer_geometry_target", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eRGBA8_UNORM, ETextureCreateFlagBits::eSamplable);
		m_pRenderInterface->CreateTextureObject("visbuffer_vboinst_target", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eD32SFLOAT, ETextureCreateFlagBits::eSamplable);
		m_pRenderInterface->CreateTextureObject("zbuffer_depth_target", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(use_low_precision_velocity ? EApiDataFormat::eRG8_UNORM : EApiDataFormat::eRG16_SFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		m_pRenderInterface->CreateTextureObject("motion_vectors_target", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eR11G11B10_UFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		m_pRenderInterface->CreateTextureObject("hdr0", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eR11G11B10_UFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		m_pRenderInterface->CreateTextureObject("hdr1", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eR11G11B10_UFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		m_pRenderInterface->CreateTextureObject("hdr0_history", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eR11G11B10_UFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		texture_ci.SetSizeScale(0.5f);
		m_pRenderInterface->CreateTextureObject("hdr0_div2", texture_ci);
	}
	
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eR11G11B10_UFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		texture_ci.SetSizeScale(0.5f);
		texture_ci.mipLevels = bloom_blur_depth;
		m_pRenderInterface->CreateTextureObject("bloom_target", texture_ci);
	}
	
	// Eye adaptation targets
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eR32_SFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		texture_ci.SetFixedSize2D(1);
		m_pRenderInterface->CreateTextureObject("avg_brightness_target", texture_ci);
	}
	
	{
		FBufferCreateInfo buffer_ci{};
		buffer_ci.flags = EBufferUsageFlagBits::eStorageBuffer;
		buffer_ci.size = 4;
		buffer_ci.count = 256;
		m_pRenderInterface->CreateBufferObject("histogram_buffer", buffer_ci);
	}

	// Some test things
	{
		FTextureCreateInfo texture_ci = MakeTexture2DCI(EApiDataFormat::eRGBA16_SFLOAT, ETextureCreateFlagBits::eSamplable | ETextureCreateFlagBits::eStorage);
		texture_ci.SetSizeScale(1.0f);
		m_pRenderInterface->CreateTextureObject("frequency_target", texture_ci);
	}

	{
		FBufferCreateInfo buffer_ci{};
		buffer_ci.flags = EBufferUsageFlagBits::eIndirectBuffer;
		buffer_ci.size = 12;
		buffer_ci.count = 3;
		m_pRenderInterface->CreateBufferObject("clusterized_command_buffer", buffer_ci);
	}

	{
		FBufferCreateInfo buffer_ci{};
		buffer_ci.flags = EBufferUsageFlagBits::eIndirectBuffer;
		buffer_ci.size = 4;
		buffer_ci.count = ceil(1922 / 8) * ceil(1082 / 8);
		m_pRenderInterface->CreateBufferObject("high_frequency_tiles", buffer_ci);
		m_pRenderInterface->CreateBufferObject("low_frequency_tiles", buffer_ci);
	}
	
	// Present image targets
	SetBackbufferResourceName("present_target");
	
	
	{
		FRenderGraphComputePassNodeCreateInfo create_info;
		create_info.name = "generate_brdflut";
		create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "lighting:brgflut_generator");
		create_info.pass_ci.oneTime = true;
	
		create_info.pass_ci.SetWorkgroupSizeByRT("brdf_lut_target", true);
		create_info.pass_ci.BindWriteResource("out_color", "brdf_lut_target");
	
		AddRenderNode(create_info);
	}
	
	{
		FRenderGraphComputePassNodeCreateInfo create_info;
		create_info.name = "generate_irradiance_cube";
		create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "lighting:irradiancecube_generator");
		create_info.pass_ci.oneTime = true;
	
		create_info.pass_ci.SetWorkgroupSizeByRT("irradiance_cube_target", true);
		create_info.pass_ci.BindWriteResource("out_color", "irradiance_cube_target");
		create_info.pass_ci.BindReadResource("sampler_color", "CasualDay4K");
	
		AddRenderNode(create_info);
	}
	
	{
		FRenderGraphComputePassNodeCreateInfo create_info;
		create_info.name = "generate_prefiltered_cube";
		create_info.pass_ci = makeComputePassCI(EComputeType::eCubemap, EQueueTypeFlagBits::eGraphics, "lighting:prefilteredmap_generator");
		create_info.pass_ci.oneTime = true;
	
		create_info.pass_ci.SetWorkgroupSizeByRT("prefiltered_cube_target", true);
		create_info.pass_ci.BindWriteResource("out_color", "prefiltered_cube_target");
		create_info.pass_ci.BindReadResource("sampler_color", "CasualDay4K");
	
		AddRenderNode(create_info);
	}
	
	// Culling(culling is temporary)
	{
		FRenderGraphComputePassNodeCreateInfo create_info;
		create_info.name = "clear_counters_pass";
		create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "culling:clear_counters");
	
		create_info.pass_ci.SetWorkgroupSizeByBuffer("IndirectCountBuffer", true);
		create_info.pass_ci.AddWriteResource("IndirectCountBuffer");
	
		AddRenderNode(create_info);
	}
	
	{
		FRenderGraphComputePassNodeCreateInfo create_info;
		create_info.name = "gpu_culling_pass";
		create_info.pass_ci = makeComputePassCI(EComputeType::eCulling, EQueueTypeFlagBits::eGraphics, "culling:cull_and_batch");
	
		create_info.pass_ci.AddWriteResource("GeometryBatches");
		create_info.pass_ci.AddWriteResource("IndexedDrawIndirectBuffer");
		create_info.pass_ci.AddWriteResource("IndirectCountBuffer");
	
		AddRenderNode(create_info);
	}
	
	// Shadow passes
	{
		FRenderGraphRenderPassNodeCreateInfo create_info;
		create_info.name = "cascaded_shadow_map_pass";
		create_info.pass_ci = makeGraphicsPassCI(ERenderingType::eCascadedShadowMap, EQueueTypeFlagBits::eGraphics, "lighting:unidirectional_shadow:CASCADED_SHADOW");
		create_info.pass_ci.SetDepthTarget("cascaded_shadows_target");
	
		create_info.pass_ci.AddReadResource("GeometryBatches");
		create_info.pass_ci.AddReadResource("IndexedDrawIndirectBuffer");
		create_info.pass_ci.AddReadResource("IndirectCountBuffer");
	
		AddRenderNode(create_info);
	}
	
	{
		FRenderGraphRenderPassNodeCreateInfo create_info;
		create_info.name = "spot_shadow_map_pass";
		create_info.pass_ci = makeGraphicsPassCI(ERenderingType::eDirectionalShadowMap, EQueueTypeFlagBits::eGraphics, "lighting:unidirectional_shadow");
		create_info.pass_ci.SetDepthTarget("directional_shadows_target");
	
		create_info.pass_ci.AddReadResource("GeometryBatches");
		create_info.pass_ci.AddReadResource("IndexedDrawIndirectBuffer");
		create_info.pass_ci.AddReadResource("IndirectCountBuffer");
	
		AddRenderNode(create_info);
	}
	
	{
		FRenderGraphRenderPassNodeCreateInfo create_info;
		create_info.name = "omni_shadow_map_pass";
		create_info.pass_ci = makeGraphicsPassCI(ERenderingType::eOmniShadowMap, EQueueTypeFlagBits::eGraphics, "lighting:unidirectional_shadow:OMNI_SHADOW");
		create_info.pass_ci.SetDepthTarget("omni_shadows_target");
	
		create_info.pass_ci.AddReadResource("GeometryBatches");
		create_info.pass_ci.AddReadResource("IndexedDrawIndirectBuffer");
		create_info.pass_ci.AddReadResource("IndirectCountBuffer");
	
		AddRenderNode(create_info);
	}
	
	// visibility buffer write
	{
		FRenderGraphRenderPassNodeCreateInfo create_info;
		create_info.name = "visibility_buffer_write_pass";
		create_info.pass_ci = makeGraphicsPassCI(ERenderingType::eGeometry, EQueueTypeFlagBits::eGraphics, "visibilitybuffer:write");
		create_info.pass_ci.SetDepthTarget("zbuffer_depth_target");
		//create_info.pass_ci.SetStencilTarget("zbuffer_depth_target");
	
		create_info.pass_ci.AddWriteResource("visbuffer_geometry_target", glm::vec4(0.f, 0.f, 0.f, 0.f));
		create_info.pass_ci.AddWriteResource("visbuffer_vboinst_target", glm::vec4(0.f, 0.f, 0.f, 0.f));
		//create_info.pass_ci.AddWriteResource("motion_vectors_target");
		create_info.pass_ci.AddReadResource("GeometryBatches");
		create_info.pass_ci.AddReadResource("IndexedDrawIndirectBuffer");
		create_info.pass_ci.AddReadResource("IndirectCountBuffer");
	
		AddRenderNode(create_info);
	}
	
	{
		FRenderGraphComputePassNodeCreateInfo create_info;
		create_info.name = "compute_materials_pass";
		create_info.pass_ci = makeComputePassCI(EComputeType::eGeometry, EQueueTypeFlagBits::eGraphics, "visibilitybuffer:shade");
		create_info.pass_ci.SetWorkgroupSizeByRT("hdr0", true);
	
		create_info.pass_ci.BindWriteResource("write_color", "hdr0");
		create_info.pass_ci.BindReadResource("prev_frame_color", "hdr0_history");
		create_info.pass_ci.BindReadResource("visbuffer_geometry", "visbuffer_geometry_target");
		create_info.pass_ci.BindReadResource("visbuffer_vboinst", "visbuffer_vboinst_target");
		create_info.pass_ci.BindReadResource("brdf_lut", "brdf_lut_target");
		create_info.pass_ci.BindReadResource("irradiance_cube", "irradiance_cube_target");
		create_info.pass_ci.BindReadResource("prefiltered_cube", "prefiltered_cube_target");
		create_info.pass_ci.BindReadResource("cascaded_shadowmap", "cascaded_shadows_target");
		create_info.pass_ci.BindReadResource("direct_shadowmap", "directional_shadows_target");
		create_info.pass_ci.BindReadResource("omni_shadowmap", "omni_shadows_target");
		create_info.pass_ci.BindReadResource("depth_buffer", "zbuffer_depth_target");
	
		create_info.pass_ci.AddReadResource("GeometryBatches");
		create_info.pass_ci.AddReadResource("IndexedDrawIndirectBuffer");
	
		AddRenderNode(create_info);
	}

	// Tile classification
	//{
	//	FRenderGraphComputePassNodeCreateInfo create_info;
	//	create_info.name = "clear_tiles_pass";
	//	create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:clear_tiles");
	//	create_info.pass_ci.SetWorkgroupSizeByBuffer("clusterized_command_buffer", true);
	//
	//	create_info.pass_ci.BindWriteResource("indirect_buffer", "clusterized_command_buffer");
	//
	//	AddRenderNode(create_info);
	//}
	//
	//{
	//	FRenderGraphComputePassNodeCreateInfo create_info;
	//	create_info.name = "classification_pass";
	//	create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:tile_freq_classification");
	//	create_info.pass_ci.SetWorkgroupSizeByRT("hdr0", true);
	//
	//	//create_info.pass_ci.BindWriteResource("write_color", "frequency_target");
	//	create_info.pass_ci.BindWriteResource("indirect_buffer", "clusterized_command_buffer");
	//	create_info.pass_ci.BindWriteResource("high_freq", "high_frequency_tiles");
	//	create_info.pass_ci.BindWriteResource("low_freq", "low_frequency_tiles");
	//	create_info.pass_ci.BindReadResource("sampler_color", "hdr0");
	//
	//	AddRenderNode(create_info);
	//}
	//
	//{
	//	FRenderGraphComputePassNodeCreateInfo create_info;
	//	create_info.name = "classification_debug_hf_pass";
	//	create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:classification_debug:HIGH_FREQUENCY");
	//
	//	create_info.pass_ci.BindWriteResource("write_color", "hdr1");
	//	create_info.pass_ci.BindReadResource("sampler_color", "hdr0");
	//	create_info.pass_ci.BindReadResource("tiles", "high_frequency_tiles");
	//	create_info.pass_ci.BindIndirectArgumentBuffer("clusterized_command_buffer", 0);
	//
	//	AddRenderNode(create_info);
	//}
	//
	//{
	//	FRenderGraphComputePassNodeCreateInfo create_info;
	//	create_info.name = "classification_debug_lf_pass";
	//	create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:classification_debug");
	//
	//	create_info.pass_ci.BindWriteResource("write_color", "hdr1");
	//	create_info.pass_ci.BindReadResource("sampler_color", "hdr0");
	//	create_info.pass_ci.BindReadResource("tiles", "low_frequency_tiles");
	//	create_info.pass_ci.BindIndirectArgumentBuffer("clusterized_command_buffer", 12);
	//
	//	AddRenderNode(create_info);
	//}
	
	// Compute static motion vectors (can be started in another queue)
	{
		FRenderGraphComputePassNodeCreateInfo create_info;
		create_info.name = "generate_motion_vectors_pass";
		create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, use_low_precision_velocity ? "postprocess:generate_motion_vectors:LOW_PRECISION_VELOCITY" : "postprocess:generate_motion_vectors");
		create_info.pass_ci.SetWorkgroupSizeByRT("zbuffer_depth_target", true);
	
		create_info.pass_ci.BindWriteResource("write_color", "motion_vectors_target");
		create_info.pass_ci.BindReadResource("depth_texture", "zbuffer_depth_target");
	
		AddRenderNode(create_info);
	}
	
	// Anti - alias step
	//{
	//	FRenderGraphComputePassNodeCreateInfo create_info;
	//	create_info.name = "compute_smaa_pass";
	//	create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:filmic_smaa_tu2x");
	//	create_info.pass_ci.SetWorkgroupSizeByRT("hdr1", true);
	//
	//	create_info.pass_ci.BindWriteResource("write_color", "hdr1");
	//	create_info.pass_ci.BindWriteResource("current_frame", "hdr0");
	//	create_info.pass_ci.BindWriteResource("prev_frame", "hdr0_history");
	//
	//	AddRenderNode(create_info);
	//}
	
	auto input_target = "hdr0";
	auto output_target = "hdr1";
	
	// TemporalAA
	//if (enable_taa)
	//{
	//	FRenderGraphComputePassNodeCreateInfo create_info;
	//	create_info.name = "resolve_temporal_aa_pass";
	//	create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:compute_temporal_aa");
	//	create_info.pass_ci.SetWorkgroupSizeByRT(next_target, true);
	//
	//	create_info.pass_ci.BindWriteResource("write_color", next_target);
	//	create_info.pass_ci.BindWriteResource("current_color", last_target);
	//	create_info.pass_ci.BindWriteResource("prev_color", "hdr0_history");
	//	create_info.pass_ci.BindWriteResource("motion_vectors", "motion_vectors_target");
	//	create_info.pass_ci.BindWriteResource("depth_image", "zbuffer_depth_target");
	//
	//	AddRenderNode(create_info);
	//
	//	std::swap(input_target, output_target);
	//}
	
	// Copy history
	{
		FRenderGraphComputePassNodeCreateInfo create_info;
		create_info.name = "copy_final_color_pass";
		create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "utility:copy_compute");
		create_info.pass_ci.SetWorkgroupSizeByRT(input_target, true);
	
		create_info.pass_ci.BindReadResource("sampler_color", input_target);
		create_info.pass_ci.BindWriteResource("write_color", "hdr0_history");
	
		AddRenderNode(create_info);
	}
	
	std::string final_target = input_target;
	
	// Post processing
	{
		u32 pass_count{ 0u };
	
		// FXAA 
		{
			FRenderGraphComputePassNodeCreateInfo create_info;
			create_info.name = "apply_fxaa";
			create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:apply_effect:FXAA");
			create_info.pass_ci.SetWorkgroupSizeByRT(input_target, true);
	
			create_info.pass_ci.BindWriteResource("write_color", output_target);
			create_info.pass_ci.BindReadResource("sampler_color", input_target);

			FRenderGraphDynamicConditionNodeCreateInfo condition_info;
			condition_info.condition = [enable_fxaa]() { return enable_fxaa; };
			condition_info.pass = CreateRenderNode(create_info);
	
			if(enable_fxaa)
				SWAP_COUNT(input_target, output_target);
		}
	
		// Compute motion blur
		{
			// Apply motion blur
			{
				FRenderGraphComputePassNodeCreateInfo create_info;
				create_info.name = "apply_motion_blur";
				create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, use_low_precision_velocity ? "postprocess:apply_effect:MOTION_BLUR:LOW_PRECISION_VELOCITY" : "postprocess:apply_effect:MOTION_BLUR");
				create_info.pass_ci.SetWorkgroupSizeByRT(input_target, true);
	
				create_info.pass_ci.BindReadResource("sampler_color", input_target);
				create_info.pass_ci.BindReadResource("motion_vectors", "motion_vectors_target");
				create_info.pass_ci.BindWriteResource("write_color", output_target);

				FRenderGraphDynamicConditionNodeCreateInfo condition_info;
				condition_info.condition = [motion_blur_enabled]() { return motion_blur_enabled; };
				condition_info.pass = CreateRenderNode(create_info);
				AddDynamicConditionNode(condition_info);
			}
	
			if(motion_blur_enabled)
				SWAP_COUNT(input_target, output_target);
		}
	
		// Compute dof
		{
			FRenderGraphGroupNodeCreateInfo dof_group{};
			dof_group.name = "compute_depth_of_field";
			dof_group.color = 0xffff33ff;
	
			// Compute bokeh
			{
				FRenderGraphComputePassNodeCreateInfo create_info;
				create_info.name = "compute_bokeh_blur_pass";
				create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:bokeh");
				create_info.pass_ci.SetWorkgroupSizeByRT(input_target, true);
	
				create_info.pass_ci.BindWriteResource("write_color", "hdr0_div2");
				create_info.pass_ci.BindReadResource("sampler_color", input_target);
	
				dof_group.nodes.emplace_back(CreateRenderNode(create_info));
			}
	
			// Apply dof
			{
				FRenderGraphComputePassNodeCreateInfo create_info;
				create_info.name = "apply_bokeh_blur_pass";
				create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:apply_effect:DOF");
				create_info.pass_ci.SetWorkgroupSizeByRT(input_target, true);
	
				create_info.pass_ci.BindWriteResource("write_color", output_target);
				create_info.pass_ci.BindReadResource("sampler_color", input_target);
				create_info.pass_ci.BindReadResource("depth_image", "zbuffer_depth_target");
				create_info.pass_ci.BindReadResource("bokeh_color", "hdr0_div2");
	
				dof_group.nodes.emplace_back(CreateRenderNode(create_info));
			}

			FRenderGraphDynamicConditionNodeCreateInfo condition_info;
			condition_info.condition = [depth_of_field_enabled]() { return depth_of_field_enabled; };
			condition_info.pass = CreateGroupNode(dof_group);
			AddDynamicConditionNode(condition_info);
	
			if(depth_of_field_enabled)
				SWAP_COUNT(input_target, output_target);
		}
	
		// Compute bloom
		{
			FRenderGraphGroupNodeCreateInfo bloom_group{};
			bloom_group.name = "compute_bloom";
			bloom_group.color = 0xffff33ff;
	
			// Starting from brightness detection pass
			{
				FRenderGraphComputePassNodeCreateInfo create_info;
				create_info.name = "brightdetect_pass";
				create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, bloom_use_brightness ? "postprocess:brightdetect" : "utility:copy_compute");
				create_info.pass_ci.SetWorkgroupSizeByRT("hdr0_div2", true);
	
				create_info.pass_ci.BindReadResource("sampler_color", input_target);
				create_info.pass_ci.BindWriteResource("write_color", "hdr0_div2");
	
				bloom_group.nodes.emplace_back(CreateRenderNode(create_info));
			}
	
			// Begin downsampling
			{
				FRenderGraphGroupNodeCreateInfo downsample_group{};
				downsample_group.name = "downsample";
				downsample_group.color = 0xff3333ff;

				FRenderGraphComputePassNodeCreateInfo create_info;
				create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:downsample:FIRST_PASS");
				create_info.pass_ci.BindReadResource("sampler_color", "hdr0_div2");
	
				for (u32 idx = 0; idx < bloom_blur_depth; ++idx)
				{
					create_info.pass_ci.SetWorkgroupSizeByRT("bloom_target", idx, true);
					create_info.pass_ci.BindWriteResource("write_color", "bloom_target", idx);
					create_info.name = std::format("downnsample_pass_{}", idx);

					downsample_group.nodes.emplace_back(CreateRenderNode(create_info));
	
					create_info.pass_ci.ClearResource();
					create_info.pass_ci.BindReadResource("sampler_color", "bloom_target", idx);
	
					if(idx == 0u)
						create_info.pass_ci.shader = "postprocess:downsample";
				}

				bloom_group.nodes.emplace_back(CreateGroupNode(downsample_group));
			}
	
			// And upsampling
			{
				FRenderGraphGroupNodeCreateInfo upsample_group{};
				upsample_group.name = "upsample";
				upsample_group.color = 0xff6633ff;

				for (u32 idx = bloom_blur_depth - 1; idx > 0; --idx)
				{
					FRenderGraphComputePassNodeCreateInfo create_info;
					create_info.name = std::format("upnsample_pass_{}", idx);
					create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:upsample");
					create_info.pass_ci.SetWorkgroupSizeByRT("bloom_target", idx - 1, true);
	
					create_info.pass_ci.BindReadResource("sampler_color", "bloom_target", idx);
					create_info.pass_ci.BindWriteResource("write_color", "bloom_target", idx - 1);
	
					upsample_group.nodes.emplace_back(CreateRenderNode(create_info));
				}

				bloom_group.nodes.emplace_back(CreateGroupNode(upsample_group));
			}
	
			// Apply bloom
			{
				FRenderGraphComputePassNodeCreateInfo create_info;
				create_info.name = "apply_bloom";
				create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:apply_effect:BLOOM");
				create_info.pass_ci.SetWorkgroupSizeByRT(output_target, true);
	
				create_info.pass_ci.BindWriteResource("write_color", output_target);
				create_info.pass_ci.BindReadResource("sampler_color", input_target);
				create_info.pass_ci.BindReadResource("bloom_color", "bloom_target", 0);
				create_info.pass_ci.BindReadResource("dirt_mask", "DirtMaskTextureExample");
	
				bloom_group.nodes.emplace_back(CreateRenderNode(create_info));
			}

			FRenderGraphDynamicConditionNodeCreateInfo condition_info;
			condition_info.condition = [bloom_enabled]() { return bloom_enabled; };
			condition_info.pass = CreateGroupNode(bloom_group);
			AddDynamicConditionNode(condition_info);
	
			if(bloom_enabled)
				SWAP_COUNT(input_target, output_target);
		}
	
		// Compute chromatic aberration
		{
			FRenderGraphComputePassNodeCreateInfo create_info;
			create_info.name = "apply_chromatic_aberration_pass";
			create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:apply_effect:CHROMATIC_ABERRATION");
			create_info.pass_ci.SetWorkgroupSizeByRT(input_target, true);
	
			create_info.pass_ci.BindWriteResource("write_color", output_target);
			create_info.pass_ci.BindReadResource("sampler_color", input_target);
	
			FRenderGraphDynamicConditionNodeCreateInfo condition_info;
			condition_info.condition = [chromatic_aberration_enabled]() { return chromatic_aberration_enabled; };
			condition_info.pass = CreateRenderNode(create_info);
			AddDynamicConditionNode(condition_info);
		
			if(chromatic_aberration_enabled)
				SWAP_COUNT(input_target, output_target);
		}
	
		// Tonemap pass
		{
			FRenderGraphGroupNodeCreateInfo tonemap_group{};
			tonemap_group.name = "compute_tonemap";
			tonemap_group.color = 0x336633ff;
	
			// Compute histogram
			{
				FRenderGraphComputePassNodeCreateInfo create_info;
				create_info.name = "compute_histogram_pass";
				create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:compute_histogram");
				create_info.pass_ci.SetWorkgroupSizeByRT(input_target, true);
	
				create_info.pass_ci.BindReadResource("sampler_color", input_target);
				create_info.pass_ci.BindWriteResource("Histogram", "histogram_buffer");
	
				tonemap_group.nodes.emplace_back(CreateRenderNode(create_info));
			}
	
			// Compute avg screen space brightness
			{
				FRenderGraphComputePassNodeCreateInfo create_info;
				create_info.name = "compute_brightness_pass";
				create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:compute_brightness");
				create_info.pass_ci.SetWorkgroupSizeManual(1, 1, 1, false);
	
				create_info.pass_ci .BindWriteResource("target", "avg_brightness_target");
				create_info.pass_ci .BindReadResource("Histogram", "histogram_buffer");
	
				tonemap_group.nodes.emplace_back(CreateRenderNode(create_info));
			}
	
			// Apply tonemapping
			{
				FRenderGraphComputePassNodeCreateInfo create_info;
				create_info.name = "apply_tonemap_pass";
				create_info.pass_ci = makeComputePassCI(EComputeType::eDefault, EQueueTypeFlagBits::eGraphics, "postprocess:apply_effect:TONEMAP");
				create_info.pass_ci.SetWorkgroupSizeByRT(input_target, true);
	
				create_info.pass_ci.BindWriteResource("write_color", output_target);
				create_info.pass_ci.BindReadResource("sampler_color", input_target);
				create_info.pass_ci.BindReadResource("avg_luminosity", "avg_brightness_target");
	
				tonemap_group.nodes.emplace_back(CreateRenderNode(create_info));
			}

			FRenderGraphDynamicConditionNodeCreateInfo condition_info;
			condition_info.condition = [tonemap_enabled]() { return tonemap_enabled; };
			condition_info.pass = CreateGroupNode(tonemap_group);
			AddDynamicConditionNode(condition_info);
	
			if(tonemap_enabled)
				SWAP_COUNT(input_target, output_target);
		}
	
		final_target = pass_count % 2 ? output_target : input_target;
	}
	
	// Present pass
	{
		FRenderGraphRenderPassNodeCreateInfo create_info;
		create_info.name = "finalize_image_pass";
		create_info.pass_ci = makeGraphicsPassCI(ERenderingType::eFullscreen, EQueueTypeFlagBits::eGraphics, "testing:screenspace");
		create_info.pass_ci.FlipFiewport();
	
		create_info.pass_ci.BindReadResource("input_image", final_target);
		//create_info.pass_ci.create_info.BindReadResource("input_image", "frequency_target");
		create_info.pass_ci.AddWriteResource("present_target");
	
		AddRenderNode(create_info);
	}

	// Debug draw pass
	{
		FRenderGraphRenderPassNodeCreateInfo create_info;
		create_info.name = "debug_draw_pass";
		create_info.pass_ci = makeGraphicsPassCI(ERenderingType::eDebugDraw, EQueueTypeFlagBits::eGraphics, "testing:debug_draw");
		create_info.pass_ci.FlipFiewport();
		create_info.pass_ci.AddWriteResource("present_target");
	
		AddRenderNode(create_info);
	}

	// ImGui pass
	{
		FRenderGraphRenderPassNodeCreateInfo create_info;
		create_info.name = "imgui_pass";
		create_info.pass_ci = makeGraphicsPassCI(ERenderingType::eImGui, EQueueTypeFlagBits::eGraphics, "testing:imgui");
		create_info.pass_ci.AddWriteResource("present_target");

		//AddRenderNode(create_info);
	}
}


void CRenderGraph::AddRenderResource(const std::string& name, const FRenderResource& resource_desc)
{
	m_nameToResource[name] = FGraphHandle(m_vRenderResources.size());
	m_vRenderResources.emplace_back(resource_desc);
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
	else if (render_pass_ci.type == ERenderingType::eDebugDraw)
		new_pass = std::make_unique<CDebugDrawPass>(m_pRenderInterface);
	else if(render_pass_ci.type == ERenderingType::eImGui)
		new_pass = std::make_unique<CImGuiPass>(m_pRenderInterface);

	if (!render_pass_ci.depthResource.empty())
		pass_ci.depthTarget = GetRenderRersourceId(render_pass_ci.depthResource);

	if (!render_pass_ci.stencilResource.empty())
		pass_ci.depthTarget = GetRenderRersourceId(render_pass_ci.stencilResource);

	new_pass->Init(pass_ci);
	m_vRenderPasses.emplace_back(std::move(new_pass));

	return pass_index;
}

u64 CRenderGraph::AddRenderPass(const std::string& name, const FRenderConfigComputeCreateInfo& compute_pass_ci)
{
	log_info("[Render Graph] Add compute pass: {}", name);

	u64 pass_index = m_vRenderPasses.size();

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

	pass_ci.groupInfo.type = compute_pass_ci.groupInfo.type;
	pass_ci.groupInfo.divideByLocals = compute_pass_ci.groupInfo.divideByLocals;
	pass_ci.groupInfo.target = GetRenderRersourceId(compute_pass_ci.groupInfo.target);
	pass_ci.groupInfo.width = compute_pass_ci.groupInfo.width;
	pass_ci.groupInfo.height = compute_pass_ci.groupInfo.height;
	pass_ci.groupInfo.depth = compute_pass_ci.groupInfo.depth;
	pass_ci.groupInfo.subresource = compute_pass_ci.groupInfo.subresource;

	new_pass->Init(pass_ci);
	m_vRenderPasses.emplace_back(std::move(new_pass));

	return pass_index;
}

void CRenderGraph::AddStaticConditionNode(FRenderGraphStaticConditionNodeCreateInfo& create_info)
{
	m_vGraphNodes.emplace_back(CreateStaticConditionNode(create_info));
}

void CRenderGraph::AddDynamicConditionNode(FRenderGraphDynamicConditionNodeCreateInfo& create_info)
{
	m_vGraphNodes.emplace_back(CreateDynamicConditionNode(create_info));
}

void CRenderGraph::AddGroupNode(FRenderGraphGroupNodeCreateInfo& create_info)
{
	m_vGraphNodes.emplace_back(CreateGroupNode(create_info));
}

void CRenderGraph::AddLayerNode(FRenderGraphGroupNodeCreateInfo& create_info)
{
	m_nameToLayer.emplace(utl::fnv1a_64_hash_cstr(create_info.name.c_str()), m_vGraphNodes.size());
	AddGroupNode(create_info);
}

void CRenderGraph::AddRenderNode(FRenderGraphRenderPassNodeCreateInfo& create_info)
{
	m_vGraphNodes.emplace_back(CreateRenderNode(create_info));
}

void CRenderGraph::AddRenderNode(FRenderGraphComputePassNodeCreateInfo& create_info)
{
	m_vGraphNodes.emplace_back(CreateRenderNode(create_info));
}


FRenderGraphNodeHeader* CRenderGraph::CreateStaticConditionNode(FRenderGraphStaticConditionNodeCreateInfo& create_info)
{
	auto* header = allocate_graph_node<FRenderGraphStaticConditionNode>(ERenderGraphNodeType::eStaticCondition);
	auto* static_condition = cast_graph_node<FRenderGraphStaticConditionNode>(header);

	if (create_info.condition)
	{
		static_condition->node = std::move(create_info.pass);
		deallocate_graph_node(&create_info.fail);
	}
	else
	{
		static_condition->node = std::move(create_info.fail);
		deallocate_graph_node(&create_info.pass);
	}
	
	return header;
}

FRenderGraphNodeHeader* CRenderGraph::CreateDynamicConditionNode(FRenderGraphDynamicConditionNodeCreateInfo& create_info)
{
	auto* header = allocate_graph_node<FRenderGraphDynamicConditionNode>(ERenderGraphNodeType::eDynamicCondition);
	auto* dynamic_condition = cast_graph_node<FRenderGraphDynamicConditionNode>(header);

	dynamic_condition->condition = std::move(create_info.condition);
	dynamic_condition->pass = std::move(create_info.pass);
	dynamic_condition->fail = std::move(create_info.fail);

	return header;
}

FRenderGraphNodeHeader* CRenderGraph::CreateGroupNode(FRenderGraphGroupNodeCreateInfo& create_info)
{
	auto* header = allocate_graph_node<FRenderGraphGroupNode>(ERenderGraphNodeType::eGroupNode);
	auto* group = cast_graph_node<FRenderGraphGroupNode>(header);

	group->name = std::move(create_info.name);
	group->color = create_info.color;
	group->nodes = std::move(create_info.nodes);
	group->queue = create_info.queue;
	
	return header;
}

FRenderGraphNodeHeader* CRenderGraph::CreateRenderNode(FRenderGraphRenderPassNodeCreateInfo& create_info)
{
	auto* header = allocate_graph_node<FRenderGraphPassNode>(ERenderGraphNodeType::ePassNode);
	auto* pass = cast_graph_node<FRenderGraphPassNode>(header);

	pass->passIndex = AddRenderPass(create_info.name, create_info.pass_ci);

	return header;
}

FRenderGraphNodeHeader* CRenderGraph::CreateRenderNode(FRenderGraphComputePassNodeCreateInfo& create_info)
{
	auto* header = allocate_graph_node<FRenderGraphPassNode>(ERenderGraphNodeType::ePassNode);
	auto* pass = cast_graph_node<FRenderGraphPassNode>(header);

	pass->passIndex = AddRenderPass(create_info.name, create_info.pass_ci);

	return header;
}

void CRenderGraph::AddPassToLayer(const std::string& layer_name, std::unique_ptr<IBasePass>&& pass)
{
	auto hash = utl::fnv1a_64_hash_cstr(layer_name.c_str());

	if (auto found = m_nameToLayer.find(hash); found != m_nameToLayer.end())
	{
		auto* header = allocate_graph_node<FRenderGraphPassNode>(ERenderGraphNodeType::ePassNode);
		auto* pass_node = cast_graph_node<FRenderGraphPassNode>(header);

		pass_node->passIndex = m_vRenderPasses.size();

		auto* layer_header = m_vGraphNodes[found->second];
		auto* layer_node = cast_graph_node<FRenderGraphGroupNode>(header);
		layer_node->nodes.emplace_back();

		m_vRenderPasses.emplace_back(std::move(pass));
	}
}

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

	// Build render passes
	EvaluateRenderGraph(ERenderGraphEvaluateFlagBits::eUpdateGraph);

	for (u64 pass_index : m_graph.passes)
	{
		auto& pass = m_vRenderPasses[pass_index];
		auto& build_info = m_graph.buildInfos[pass_index];
		pass->Build(build_info);
	}	
}

void CRenderGraph::PostLoad()
{
	for (u64 pass_index : m_graph.passes)
	{
		auto& pass = m_vRenderPasses[pass_index];
		pass->PostLoad();
	}
}

void CRenderGraph::Update(f32 dt)
{
	for (u64 pass_index : m_graph.passes)
	{
		auto& pass = m_vRenderPasses[pass_index];
		pass->Update(dt);
	}
}

void CRenderGraph::Execute()
{
	auto& render_context = m_pRenderInterface->GetRenderContext();

	EvaluateRenderGraph(ERenderGraphEvaluateFlagBits::eRender);

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

	// Indirect buffers
	if (!raw_pass_ci->argumentBuffer.empty())
	{
		auto resource_id = GetRenderRersourceId(raw_pass_ci->argumentBuffer);
		auto& render_resource = GetRenderResource(resource_id);

		if (render_resource.type != ERenderResourceTypeFlagBits::eBuffer)
			log_error("[Render Graph] Non buffer resource is set as indirect buffer.");

		// Indirect beffer is read here in any case
		auto resource_key = packResourceGroup(ERenderResourceAccessFlagBits::eRead, ERenderResourceGroupFlagBits::eIndirectArgument);
		pass_ci->resources[resource_key].emplace_back(FRenderResourceBinding{ resource_id, utl::cast<u32>(raw_pass_ci->argumentOffset) });
		AapplySingleRenderResourceDependencies(resource_id, ERenderResourceAccessFlagBits::eRead, pass_index, pass_ci->queue);
	}

	if (!raw_pass_ci->countBuffer.empty())
	{
		auto resource_id = GetRenderRersourceId(raw_pass_ci->countBuffer);
		auto& render_resource = GetRenderResource(resource_id);

		if (render_resource.type != ERenderResourceTypeFlagBits::eBuffer)
			log_error("[Render Graph] Non buffer resource is set as indirect buffer.");

		// Indirect beffer is read here in any case
		auto resource_key = packResourceGroup(ERenderResourceAccessFlagBits::eRead, ERenderResourceGroupFlagBits::eIndirectCount);
		pass_ci->resources[resource_key].emplace_back(FRenderResourceBinding{ resource_id, utl::cast<u32>(raw_pass_ci->countOffset) });
		AapplySingleRenderResourceDependencies(resource_id, ERenderResourceAccessFlagBits::eRead, pass_index, pass_ci->queue);
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
	//static std::hash<u64> hasher;
	//u64 new_hash{ 0ull };
	//for (u64 pass_index : m_vPassExecutionQueue)
	//{
	//	auto& pass = m_vRenderPasses[pass_index];
	//
	//	if (pass->CanExecute())
	//		new_hash ^= hasher(pass_index);
	//}
	//
	//bool res{ new_hash != m_executionHash };
	//m_executionHash = new_hash;
	return true;
}
