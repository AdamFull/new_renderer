#include <RenderConfig.h>

#include <angelscript.h>
#include <scriptstdvector/scriptstdvector.h>
#include <scriptstdstring/scriptstdstring.h>

#include <logger/logger.h>

using namespace engine::render;

template<class _Ty>
_Ty* RenderConfig_CopyFactory(const _Ty& other) {
	return new _Ty(other);
}

void RenderConfig_MessageCallback(const asSMessageInfo* msg, void* param) {
	const char* typeStr = "";

	switch (msg->type)
	{
	case asMSGTYPE_ERROR: log_error("{}({},{}): {}", msg->section, msg->row, msg->col, msg->message); break;
	case asMSGTYPE_WARNING: log_warning("{}({},{}): {}", msg->section, msg->row, msg->col, msg->message); break;
	case asMSGTYPE_INFORMATION: log_info("{}({},{}): {}", msg->section, msg->row, msg->col, msg->message); break;
	}
}

CRenderScriptExecutor::~CRenderScriptExecutor()
{
	m_pContext->Release();
}

int CRenderScriptExecutor::run(size_t function)
{
	auto found_function = m_scriptFunctions.find(function);
	if (found_function != m_scriptFunctions.end())
	{
		m_pContext->Unprepare();
		m_pContext->Prepare(found_function->second);
		return m_pContext->Execute();
	}

	return -1;
}

void CRenderScriptCore::create(IRenderInterface* render_interface, IRenderHandleInterface* render_context)
{
	int res{ 0 };

	m_pScriptEngine = asCreateScriptEngine();

	m_pScriptEngine->SetMessageCallback(asFUNCTION(RenderConfig_MessageCallback), nullptr, asCALL_CDECL);

	registerEnums();
	registerObjects();
	registerGlobals(render_interface, render_context);
}

void CRenderScriptCore::build(const std::unordered_map<std::string, std::vector<uint8_t>>& programs)
{
	for (auto& [name, code] : programs)
	{
		auto script_executor = std::make_unique<CRenderScriptExecutor>();

		auto script_name = name.c_str();
		script_executor->m_pScriptModule = m_pScriptEngine->GetModule(script_name, asGM_ALWAYS_CREATE);
		script_executor->m_pScriptModule->AddScriptSection(script_name, reinterpret_cast<const char*>(code.data()), code.size());
		script_executor->m_pScriptModule->Build();

		script_executor->m_pContext = m_pScriptEngine->CreateContext();

		uint32_t function_count = script_executor->m_pScriptModule->GetFunctionCount();
		for (uint32_t idx = 0u; idx < function_count; ++idx)
		{
			asIScriptFunction* function = script_executor->m_pScriptModule->GetFunctionByIndex(idx);
			auto function_hashname = utl::fnv1a_64_hash(function->GetDeclaration(true, true, true));
			script_executor->m_scriptFunctions.emplace(function_hashname, function);
		}

		auto script_hashname = utl::fnv1a_64_hash(script_name);
		m_scriptExecutors.emplace(script_hashname, std::move(script_executor));
	}
}

int CRenderScriptCore::execute(const char* script_name, const char* function)
{
	auto script_hashname = utl::fnv1a_64_hash(script_name);
	auto found_executor = m_scriptExecutors.find(script_hashname);
	if (found_executor != m_scriptExecutors.end())
		return found_executor->second->run(utl::fnv1a_64_hash(function));

	return -1;
}

//void CRenderScriptCore::executeOnce(const std::string& name, const std::vector<uint8_t>& script)
//{
//	asIScriptModule* mod = m_pScriptEngine->GetModule(name.c_str(), asGM_ALWAYS_CREATE);
//	mod->AddScriptSection(name.c_str(), reinterpret_cast<const char*>(script.data()), script.size());
//	mod->Build();
//
//	auto main_func = mod->GetFunctionByDecl("void main()");
//	if (main_func)
//	{
//		auto ctx = m_pScriptEngine->CreateContext();
//		ctx->Prepare(main_func);
//		auto res = ctx->Execute();
//		if (res != asEXECUTION_FINISHED)
//		{
//			ctx->Release();
//			return;
//		}
//
//		ctx->Release();
//	}
//
//	m_pScriptEngine->DiscardModule(name.c_str());
//}

void CRenderScriptCore::registerEnums()
{
	int32_t res{ 0 };

	// Register ETextureFormat
	res = m_pScriptEngine->RegisterEnum("ETextureFormat");
	res = m_pScriptEngine->RegisterEnumValue("ETextureFormat", "FORMAT_RG16_SFLOAT", FORMAT_RG16_SFLOAT);
	res = m_pScriptEngine->RegisterEnumValue("ETextureFormat", "FORMAT_RGBA8_SRGB", FORMAT_RGBA8_SRGB);
	res = m_pScriptEngine->RegisterEnumValue("ETextureFormat", "FORMAT_RGBA8_UNORM", FORMAT_RGBA8_UNORM);
	res = m_pScriptEngine->RegisterEnumValue("ETextureFormat", "FORMAT_RGBA16_SFLOAT", FORMAT_RGBA16_SFLOAT);
	res = m_pScriptEngine->RegisterEnumValue("ETextureFormat", "FORMAT_RGBA32_SFLOAT", FORMAT_RGBA32_SFLOAT);

	// Register EResourceTypeBits
	res = m_pScriptEngine->RegisterEnum("EResourceTypeBits");
	res = m_pScriptEngine->RegisterEnumValue("EResourceTypeBits", "RESOURCE_TYPE_TEXTURE_BIT", RESOURCE_TYPE_TEXTURE_BIT);
	res = m_pScriptEngine->RegisterEnumValue("EResourceTypeBits", "RESOURCE_TYPE_RENDER_TARGET_BIT", RESOURCE_TYPE_RENDER_TARGET_BIT);
	res = m_pScriptEngine->RegisterEnumValue("EResourceTypeBits", "RESOURCE_TYPE_BUFFER_BIT", RESOURCE_TYPE_BUFFER_BIT);
	res = m_pScriptEngine->RegisterEnumValue("EResourceTypeBits", "RESOURCE_TYPE_SHADER_BIT", RESOURCE_TYPE_SHADER_BIT);

	// Register ETextureImageTypeFlagBits
	res = m_pScriptEngine->RegisterEnum("ETextureImageTypeFlagBits");
	res = m_pScriptEngine->RegisterEnumValue("ETextureImageTypeFlagBits", "TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT", TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT);
	res = m_pScriptEngine->RegisterEnumValue("ETextureImageTypeFlagBits", "TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT", TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT);
	res = m_pScriptEngine->RegisterEnumValue("ETextureImageTypeFlagBits", "TEXTURE_IMAGE_TYPE_1D_FLAG_BIT", TEXTURE_IMAGE_TYPE_1D_FLAG_BIT);
	res = m_pScriptEngine->RegisterEnumValue("ETextureImageTypeFlagBits", "TEXTURE_IMAGE_TYPE_2D_FLAG_BIT", TEXTURE_IMAGE_TYPE_2D_FLAG_BIT);
	res = m_pScriptEngine->RegisterEnumValue("ETextureImageTypeFlagBits", "TEXTURE_IMAGE_TYPE_3D_FLAG_BIT", TEXTURE_IMAGE_TYPE_3D_FLAG_BIT);
	res = m_pScriptEngine->RegisterEnumValue("ETextureImageTypeFlagBits", "TEXTURE_IMAGE_TYPE_DEPTH_FLAG_BIT", TEXTURE_IMAGE_TYPE_DEPTH_FLAG_BIT);
	res = m_pScriptEngine->RegisterEnumValue("ETextureImageTypeFlagBits", "TEXTURE_IMAGE_TYPE_TRANSFERABLE_FLAG_BIT", TEXTURE_IMAGE_TYPE_TRANSFERABLE_FLAG_BIT);
	res = m_pScriptEngine->RegisterEnumValue("ETextureImageTypeFlagBits", "TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT", TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT);
	res = m_pScriptEngine->RegisterEnumValue("ETextureImageTypeFlagBits", "TEXTURE_IMAGE_TYPE_STORAGE_FLAG_BIT", TEXTURE_IMAGE_TYPE_STORAGE_FLAG_BIT);

	// Register EQueueFamilyUsageFlagBits
	res = m_pScriptEngine->RegisterEnum("EQueueFamilyUsageFlagBits");
	res = m_pScriptEngine->RegisterEnumValue("EQueueFamilyUsageFlagBits", "QUEUE_FAMILY_USAGE_PRESENT_FLAG_BIT", QUEUE_FAMILY_USAGE_PRESENT_FLAG_BIT);
	res = m_pScriptEngine->RegisterEnumValue("EQueueFamilyUsageFlagBits", "QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT", QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT);
	res = m_pScriptEngine->RegisterEnumValue("EQueueFamilyUsageFlagBits", "QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT", QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT);
	res = m_pScriptEngine->RegisterEnumValue("EQueueFamilyUsageFlagBits", "QUEUE_FAMILY_USAGE_TRANSFER_FLAG_BIT", QUEUE_FAMILY_USAGE_TRANSFER_FLAG_BIT);

	// Register ERenderingType (Not needed with render scripts)
	res = m_pScriptEngine->RegisterEnum("ERenderingType");
	res = m_pScriptEngine->RegisterEnumValue("ERenderingType", "RENDERING_TYPE_CASCADED_SHADOWS", RENDERING_TYPE_CASCADED_SHADOWS);
	res = m_pScriptEngine->RegisterEnumValue("ERenderingType", "RENDERING_TYPE_DIRECTIONAL_SHADOWS", RENDERING_TYPE_DIRECTIONAL_SHADOWS);
	res = m_pScriptEngine->RegisterEnumValue("ERenderingType", "RENDERING_TYPE_OMNI_SHADOWS", RENDERING_TYPE_OMNI_SHADOWS);
	res = m_pScriptEngine->RegisterEnumValue("ERenderingType", "RENDERING_TYPE_COMPUTE", RENDERING_TYPE_COMPUTE);
	res = m_pScriptEngine->RegisterEnumValue("ERenderingType", "RENDERING_TYPE_ZBUFFER", RENDERING_TYPE_ZBUFFER);
	res = m_pScriptEngine->RegisterEnumValue("ERenderingType", "RENDERING_TYPE_SCREENSPACE", RENDERING_TYPE_SCREENSPACE);
	res = m_pScriptEngine->RegisterEnumValue("ERenderingType", "RENDERING_TYPE_BRDFLUT_GENERATOR", RENDERING_TYPE_BRDFLUT_GENERATOR);
	res = m_pScriptEngine->RegisterEnumValue("ERenderingType", "RENDERING_TYPE_IRRADIANCE_CUBE_GENERATOR", RENDERING_TYPE_IRRADIANCE_CUBE_GENERATOR);
	res = m_pScriptEngine->RegisterEnumValue("ERenderingType", "RENDERING_TYPE_PREFILTERED_CUBE_GENERATOR", RENDERING_TYPE_PREFILTERED_CUBE_GENERATOR);
}

void CRenderScriptCore::registerObjects()
{
	int32_t res{ 0 };

	// FResourceView registration
	RegisterASType<FResourceView>(m_pScriptEngine);
	res = m_pScriptEngine->RegisterObjectProperty("FResourceView", "uint64 id", asOFFSET(FResourceView, id));
	res = m_pScriptEngine->RegisterObjectProperty("FResourceView", "uint mask", asOFFSET(FResourceView, mask));

	RegisterASType<FPoint2D<uint32_t>>(m_pScriptEngine, "FU32Point2D");
	res = m_pScriptEngine->RegisterObjectProperty("FU32Point2D", "uint x", asOFFSET(FPoint2D<uint32_t>, x));
	res = m_pScriptEngine->RegisterObjectProperty("FU32Point2D", "uint y", asOFFSET(FPoint2D<uint32_t>, y));

	RegisterStdString(m_pScriptEngine);
	RegisterStdVector<FResourceView>(m_pScriptEngine);

	// FShaderNameLoadInfo registration
	RegisterASType<FShaderNameLoadInfo>(m_pScriptEngine);
	res = m_pScriptEngine->RegisterObjectProperty("FShaderNameLoadInfo", "uint64 library", asOFFSET(FShaderNameLoadInfo, libraryId));
	res = m_pScriptEngine->RegisterObjectProperty("FShaderNameLoadInfo", "uint64 shader", asOFFSET(FShaderNameLoadInfo, shaderId));

	// FTextureCreateInfo registration
	RegisterASType<FTextureCreateInfo>(m_pScriptEngine);
	res = m_pScriptEngine->RegisterObjectProperty("FTextureCreateInfo", "bool present_image", asOFFSET(FTextureCreateInfo, isSwapchainTarget));
	res = m_pScriptEngine->RegisterObjectProperty("FTextureCreateInfo", "uint format", asOFFSET(FTextureCreateInfo, format));
	res = m_pScriptEngine->RegisterObjectProperty("FTextureCreateInfo", "uint type", asOFFSET(FTextureCreateInfo, typeFlags));
	res = m_pScriptEngine->RegisterObjectProperty("FTextureCreateInfo", "uint width", asOFFSET(FTextureCreateInfo, width));
	res = m_pScriptEngine->RegisterObjectProperty("FTextureCreateInfo", "uint height", asOFFSET(FTextureCreateInfo, height));
	res = m_pScriptEngine->RegisterObjectProperty("FTextureCreateInfo", "uint depth", asOFFSET(FTextureCreateInfo, depth));
	res = m_pScriptEngine->RegisterObjectProperty("FTextureCreateInfo", "uint mip_levels", asOFFSET(FTextureCreateInfo, mipLevels));
	res = m_pScriptEngine->RegisterObjectProperty("FTextureCreateInfo", "uint array_layers", asOFFSET(FTextureCreateInfo, arrayLayers));

	// FRenderingCreateInfo registration
	RegisterASType<FRenderingCreateInfo>(m_pScriptEngine);
	res = m_pScriptEngine->RegisterObjectProperty("FRenderingCreateInfo", "uint queue", asOFFSET(FRenderingCreateInfo, queue));
	res = m_pScriptEngine->RegisterObjectProperty("FRenderingCreateInfo", "uint type", asOFFSET(FRenderingCreateInfo, type));
	res = m_pScriptEngine->RegisterObjectProperty("FRenderingCreateInfo", "FShaderNameLoadInfo shader", asOFFSET(FRenderingCreateInfo, shader));
	res = m_pScriptEngine->RegisterObjectProperty("FRenderingCreateInfo", "bool one_time", asOFFSET(FRenderingCreateInfo, oneTime));
	res = m_pScriptEngine->RegisterObjectProperty("FRenderingCreateInfo", "uint flip_viewport", asOFFSET(FRenderingCreateInfo, flipViewport));
	res = m_pScriptEngine->RegisterObjectProperty("FRenderingCreateInfo", "vector_FResourceView input", asOFFSET(FRenderingCreateInfo, shaderInputs));
	res = m_pScriptEngine->RegisterObjectProperty("FRenderingCreateInfo", "vector_FResourceView output", asOFFSET(FRenderingCreateInfo, renderTargets));
	res = m_pScriptEngine->RegisterObjectProperty("FRenderingCreateInfo", "FResourceView depth", asOFFSET(FRenderingCreateInfo, depthTarget));
	res = m_pScriptEngine->RegisterObjectProperty("FRenderingCreateInfo", "FResourceView stencil", asOFFSET(FRenderingCreateInfo, stencilTarget));

	// IRenderInterface registration
	res = m_pScriptEngine->RegisterObjectType("IRenderInterface", 0, asOBJ_REF | asOBJ_NOCOUNT);
	res = m_pScriptEngine->RegisterObjectMethod("IRenderInterface", "FShaderNameLoadInfo getShaderNameTag(const string &in)", asMETHOD(IRenderInterface, getShaderNameTag), asCALL_THISCALL);
	res = m_pScriptEngine->RegisterObjectMethod("IRenderInterface", "void createRendering(const FRenderingCreateInfo &in)", asMETHOD(IRenderInterface, createRendering), asCALL_THISCALL);

	// IRenderHandleInterface registration
	res = m_pScriptEngine->RegisterObjectType("IRenderHandleInterface", 0, asOBJ_REF | asOBJ_NOCOUNT);
	res = m_pScriptEngine->RegisterObjectMethod("IRenderHandleInterface", "FResourceView createRenderTarget(const string &in, const FTextureCreateInfo &in)", asMETHOD(IRenderHandleInterface, createRenderTargets), asCALL_THISCALL);
	res = m_pScriptEngine->RegisterObjectMethod("IRenderHandleInterface", "FResourceView getResource(const string &in, uint resource_type)", asMETHOD(IRenderHandleInterface, getResource), asCALL_THISCALL);
	res = m_pScriptEngine->RegisterObjectMethod("IRenderHandleInterface", "FU32Point2D getRenderResolution()", asMETHOD(IRenderHandleInterface, getRenderResolution), asCALL_THISCALL);
	res = m_pScriptEngine->RegisterObjectMethod("IRenderHandleInterface", "uint getDepthFormat()", asMETHOD(IRenderHandleInterface, getDepthFormat), asCALL_THISCALL);
	res = m_pScriptEngine->RegisterObjectMethod("IRenderHandleInterface", "uint getApiFormat(uint format)", asMETHOD(IRenderHandleInterface, getApiFormat), asCALL_THISCALL);
	res = m_pScriptEngine->RegisterObjectMethod("IRenderHandleInterface", "uint getPresentFormat()", asMETHOD(IRenderHandleInterface, getPresentFormat), asCALL_THISCALL);
	
}

void CRenderScriptCore::registerGlobals(IRenderInterface* render_interface, IRenderHandleInterface* render_context)
{
	int32_t res{ 0 };
	res = m_pScriptEngine->RegisterGlobalProperty("IRenderInterface render_interface", render_interface);
	res = m_pScriptEngine->RegisterGlobalProperty("IRenderHandleInterface render_context", render_context);
}