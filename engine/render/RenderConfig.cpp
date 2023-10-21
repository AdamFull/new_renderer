#include "RenderConfig.h"

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

void CRenderConfig::initialize(IRenderInterface* render_interface, IRenderHandleInterface* render_context)
{
	scriptEngine = asCreateScriptEngine();

	int res{ 0 };
	scriptEngine->SetMessageCallback(asFUNCTION(RenderConfig_MessageCallback), nullptr, asCALL_CDECL);

	res = scriptEngine->RegisterEnum("ETextureFormat");
	res = scriptEngine->RegisterEnumValue("ETextureFormat", "FORMAT_RG16_SFLOAT", FORMAT_RG16_SFLOAT);
	res = scriptEngine->RegisterEnumValue("ETextureFormat", "FORMAT_RGBA8_SRGB", FORMAT_RGBA8_SRGB);
	res = scriptEngine->RegisterEnumValue("ETextureFormat", "FORMAT_RGBA8_UNORM", FORMAT_RGBA8_UNORM);
	res = scriptEngine->RegisterEnumValue("ETextureFormat", "FORMAT_RGBA16_SFLOAT", FORMAT_RGBA16_SFLOAT);
	res = scriptEngine->RegisterEnumValue("ETextureFormat", "FORMAT_RGBA32_SFLOAT", FORMAT_RGBA32_SFLOAT);

	res = scriptEngine->RegisterEnum("EResourceTypeBits");
	res = scriptEngine->RegisterEnumValue("EResourceTypeBits", "RESOURCE_TYPE_TEXTURE_BIT", RESOURCE_TYPE_TEXTURE_BIT);
	res = scriptEngine->RegisterEnumValue("EResourceTypeBits", "RESOURCE_TYPE_RENDER_TARGET_BIT", RESOURCE_TYPE_RENDER_TARGET_BIT);
	res = scriptEngine->RegisterEnumValue("EResourceTypeBits", "RESOURCE_TYPE_BUFFER_BIT", RESOURCE_TYPE_BUFFER_BIT);
	res = scriptEngine->RegisterEnumValue("EResourceTypeBits", "RESOURCE_TYPE_SHADER_BIT", RESOURCE_TYPE_SHADER_BIT);

	res = scriptEngine->RegisterEnum("ETextureImageTypeFlagBits");
	res = scriptEngine->RegisterEnumValue("ETextureImageTypeFlagBits", "TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT", TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT);
	res = scriptEngine->RegisterEnumValue("ETextureImageTypeFlagBits", "TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT", TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT);
	res = scriptEngine->RegisterEnumValue("ETextureImageTypeFlagBits", "TEXTURE_IMAGE_TYPE_1D_FLAG_BIT", TEXTURE_IMAGE_TYPE_1D_FLAG_BIT);
	res = scriptEngine->RegisterEnumValue("ETextureImageTypeFlagBits", "TEXTURE_IMAGE_TYPE_2D_FLAG_BIT", TEXTURE_IMAGE_TYPE_2D_FLAG_BIT);
	res = scriptEngine->RegisterEnumValue("ETextureImageTypeFlagBits", "TEXTURE_IMAGE_TYPE_3D_FLAG_BIT", TEXTURE_IMAGE_TYPE_3D_FLAG_BIT);
	res = scriptEngine->RegisterEnumValue("ETextureImageTypeFlagBits", "TEXTURE_IMAGE_TYPE_DEPTH_FLAG_BIT", TEXTURE_IMAGE_TYPE_DEPTH_FLAG_BIT);
	res = scriptEngine->RegisterEnumValue("ETextureImageTypeFlagBits", "TEXTURE_IMAGE_TYPE_TRANSFERABLE_FLAG_BIT", TEXTURE_IMAGE_TYPE_TRANSFERABLE_FLAG_BIT);
	res = scriptEngine->RegisterEnumValue("ETextureImageTypeFlagBits", "TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT", TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT);
	res = scriptEngine->RegisterEnumValue("ETextureImageTypeFlagBits", "TEXTURE_IMAGE_TYPE_STORAGE_FLAG_BIT", TEXTURE_IMAGE_TYPE_STORAGE_FLAG_BIT);

	res = scriptEngine->RegisterEnum("EQueueFamilyUsageFlagBits");
	res = scriptEngine->RegisterEnumValue("EQueueFamilyUsageFlagBits", "QUEUE_FAMILY_USAGE_PRESENT_FLAG_BIT", QUEUE_FAMILY_USAGE_PRESENT_FLAG_BIT);
	res = scriptEngine->RegisterEnumValue("EQueueFamilyUsageFlagBits", "QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT", QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT);
	res = scriptEngine->RegisterEnumValue("EQueueFamilyUsageFlagBits", "QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT", QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT);
	res = scriptEngine->RegisterEnumValue("EQueueFamilyUsageFlagBits", "QUEUE_FAMILY_USAGE_TRANSFER_FLAG_BIT", QUEUE_FAMILY_USAGE_TRANSFER_FLAG_BIT);

	res = scriptEngine->RegisterEnum("ERenderingType");
	res = scriptEngine->RegisterEnumValue("ERenderingType", "RENDERING_TYPE_CASCADED_SHADOWS", RENDERING_TYPE_CASCADED_SHADOWS);
	res = scriptEngine->RegisterEnumValue("ERenderingType", "RENDERING_TYPE_DIRECTIONAL_SHADOWS", RENDERING_TYPE_DIRECTIONAL_SHADOWS);
	res = scriptEngine->RegisterEnumValue("ERenderingType", "RENDERING_TYPE_OMNI_SHADOWS", RENDERING_TYPE_OMNI_SHADOWS);
	res = scriptEngine->RegisterEnumValue("ERenderingType", "RENDERING_TYPE_COMPUTE", RENDERING_TYPE_COMPUTE);
	res = scriptEngine->RegisterEnumValue("ERenderingType", "RENDERING_TYPE_ZBUFFER", RENDERING_TYPE_ZBUFFER);
	res = scriptEngine->RegisterEnumValue("ERenderingType", "RENDERING_TYPE_SCREENSPACE", RENDERING_TYPE_SCREENSPACE);
	res = scriptEngine->RegisterEnumValue("ERenderingType", "RENDERING_TYPE_BRDFLUT_GENERATOR", RENDERING_TYPE_BRDFLUT_GENERATOR);
	res = scriptEngine->RegisterEnumValue("ERenderingType", "RENDERING_TYPE_IRRADIANCE_CUBE_GENERATOR", RENDERING_TYPE_IRRADIANCE_CUBE_GENERATOR);
	res = scriptEngine->RegisterEnumValue("ERenderingType", "RENDERING_TYPE_PREFILTERED_CUBE_GENERATOR", RENDERING_TYPE_PREFILTERED_CUBE_GENERATOR);

	// FResourceView registration
	RegisterASType<FResourceView>(scriptEngine);
	res = scriptEngine->RegisterObjectProperty("FResourceView", "uint64 id", asOFFSET(FResourceView, id));
	res = scriptEngine->RegisterObjectProperty("FResourceView", "uint mask", asOFFSET(FResourceView, mask));

	RegisterASType<FPoint2D<uint32_t>>(scriptEngine, "FU32Point2D");
	res = scriptEngine->RegisterObjectProperty("FU32Point2D", "uint x", asOFFSET(FPoint2D<uint32_t>, x));
	res = scriptEngine->RegisterObjectProperty("FU32Point2D", "uint y", asOFFSET(FPoint2D<uint32_t>, y));

	RegisterStdString(scriptEngine);
	RegisterStdVector<FResourceView>(scriptEngine);

	// FTextureCreateInfo registration
	RegisterASType<FTextureCreateInfo>(scriptEngine);
	res = scriptEngine->RegisterObjectProperty("FTextureCreateInfo", "bool present_image", asOFFSET(FTextureCreateInfo, isSwapchainTarget));
	res = scriptEngine->RegisterObjectProperty("FTextureCreateInfo", "uint format", asOFFSET(FTextureCreateInfo, format));
	res = scriptEngine->RegisterObjectProperty("FTextureCreateInfo", "uint type", asOFFSET(FTextureCreateInfo, typeFlags));
	res = scriptEngine->RegisterObjectProperty("FTextureCreateInfo", "uint width", asOFFSET(FTextureCreateInfo, width));
	res = scriptEngine->RegisterObjectProperty("FTextureCreateInfo", "uint height", asOFFSET(FTextureCreateInfo, height));
	res = scriptEngine->RegisterObjectProperty("FTextureCreateInfo", "uint depth", asOFFSET(FTextureCreateInfo, depth));
	res = scriptEngine->RegisterObjectProperty("FTextureCreateInfo", "uint mip_levels", asOFFSET(FTextureCreateInfo, mipLevels));
	res = scriptEngine->RegisterObjectProperty("FTextureCreateInfo", "uint array_layers", asOFFSET(FTextureCreateInfo, arrayLayers));

	// FRenderingCreateInfo registration
	RegisterASType<FRenderingCreateInfo>(scriptEngine);
	res = scriptEngine->RegisterObjectProperty("FRenderingCreateInfo", "uint queue", asOFFSET(FRenderingCreateInfo, queue));
	res = scriptEngine->RegisterObjectProperty("FRenderingCreateInfo", "uint type", asOFFSET(FRenderingCreateInfo, type));
	res = scriptEngine->RegisterObjectProperty("FRenderingCreateInfo", "FResourceView shader", asOFFSET(FRenderingCreateInfo, shader));
	res = scriptEngine->RegisterObjectProperty("FRenderingCreateInfo", "bool one_time", asOFFSET(FRenderingCreateInfo, oneTime));
	res = scriptEngine->RegisterObjectProperty("FRenderingCreateInfo", "uint flip_viewport", asOFFSET(FRenderingCreateInfo, flipViewport));
	res = scriptEngine->RegisterObjectProperty("FRenderingCreateInfo", "vector_FResourceView input", asOFFSET(FRenderingCreateInfo, shaderInputs));
	res = scriptEngine->RegisterObjectProperty("FRenderingCreateInfo", "vector_FResourceView output", asOFFSET(FRenderingCreateInfo, renderTargets));
	res = scriptEngine->RegisterObjectProperty("FRenderingCreateInfo", "FResourceView depth", asOFFSET(FRenderingCreateInfo, depthTarget));

	res = scriptEngine->RegisterObjectType("IRenderInterface", 0, asOBJ_REF | asOBJ_NOCOUNT);
	res = scriptEngine->RegisterObjectMethod("IRenderInterface", "FResourceView getShaderNameTag(const string &in)", asMETHOD(IRenderInterface, getShaderNameTag), asCALL_THISCALL);
	res = scriptEngine->RegisterObjectMethod("IRenderInterface", "void createRendering(const FRenderingCreateInfo &in)", asMETHOD(IRenderInterface, createRendering), asCALL_THISCALL);
	res = scriptEngine->RegisterGlobalProperty("IRenderInterface render_interface", render_interface);

	res = scriptEngine->RegisterObjectType("IRenderHandleInterface", 0, asOBJ_REF | asOBJ_NOCOUNT);
	res = scriptEngine->RegisterObjectMethod("IRenderHandleInterface", "FResourceView createRenderTarget(const string &in, const FTextureCreateInfo &in)", asMETHOD(IRenderHandleInterface, createRenderTargets), asCALL_THISCALL);
	res = scriptEngine->RegisterObjectMethod("IRenderHandleInterface", "FResourceView getResource(const string &in, uint resource_type)", asMETHOD(IRenderHandleInterface, getResource), asCALL_THISCALL);
	res = scriptEngine->RegisterObjectMethod("IRenderHandleInterface", "FU32Point2D getRenderResolution()", asMETHOD(IRenderHandleInterface, getRenderResolution), asCALL_THISCALL);
	res = scriptEngine->RegisterObjectMethod("IRenderHandleInterface", "uint getDepthFormat()", asMETHOD(IRenderHandleInterface, getDepthFormat), asCALL_THISCALL);
	res = scriptEngine->RegisterObjectMethod("IRenderHandleInterface", "uint getApiFormat(uint format)", asMETHOD(IRenderHandleInterface, getApiFormat), asCALL_THISCALL);
	res = scriptEngine->RegisterObjectMethod("IRenderHandleInterface", "uint getPresentFormat()", asMETHOD(IRenderHandleInterface, getPresentFormat), asCALL_THISCALL);
	res = scriptEngine->RegisterGlobalProperty("IRenderHandleInterface render_context", render_context);
}

void CRenderConfig::execute(const std::vector<uint8_t>& script)
{
	asIScriptModule* mod = scriptEngine->GetModule("config_executor", asGM_ALWAYS_CREATE);
	mod->AddScriptSection("config_executor", reinterpret_cast<const char*>(script.data()), script.size());
	mod->Build();

	auto main_func = mod->GetFunctionByDecl("void main()");
	if (main_func)
	{
		auto ctx = scriptEngine->CreateContext();
		ctx->Prepare(main_func);
		auto res = ctx->Execute();
		if (res != asEXECUTION_FINISHED)
		{
			ctx->Release();
			return;
		}

		ctx->Release();
	}

	scriptEngine->DiscardModule("config_executor");
}