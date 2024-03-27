#include "RenderConfigBindings.h"

#include <engine/interface/render/RenderInterface.h>
#include <engine/render/ShaderObject.h>
#include <glm/glm.hpp>

#include <sol/sol.hpp>
#include <utility>

namespace engine
{
	namespace render
	{
		template<class _Ty>
		void register_point2d(const char* name, sol::state* lua)
		{
			using _TargetType = FPoint2D<_Ty>;

			lua->new_usertype<_TargetType>(
				name,
				sol::constructors<
				_TargetType(), _TargetType(_Ty), _TargetType(_Ty, _Ty),
				_TargetType(const _TargetType&)
				>(),
				"x", &_TargetType::x,
				"y", &_TargetType::y
			);
		}

		template<class _Ty>
		void register_rect2d(const char* name, sol::state* lua)
		{
			using _TargetType = FRect2D<_Ty>;
			using _PointType = FPoint2D<_Ty>;

			lua->new_usertype<_TargetType>(
				name,
				sol::constructors<
				_TargetType(), _TargetType(_PointType, _PointType), _TargetType(_Ty, _Ty, _Ty, _Ty),
				_TargetType(const _TargetType&)
				>(),
				"offset", &_TargetType::offset,
				"size", &_TargetType::size
			);
		}

		void bindRenderInterfaceEnums(sol::state* lua)
		{
			// Register ETextureFormat
			lua->new_enum(
				"Format",
				"rgba8_srgb", EApiDataFormat::eRGBA8_SRGB,

				"r8_unorm", EApiDataFormat::eR8_UNORM,
				"rg8_unorm", EApiDataFormat::eRG8_UNORM,
				"rgb8_unorm", EApiDataFormat::eRGB8_UNORM,
				"rgba8_unorm", EApiDataFormat::eRGBA8_UNORM,

				"r16_unorm", EApiDataFormat::eR16_UNORM,
				"rg16_unorm", EApiDataFormat::eRG16_UNORM,
				"rgb16_unorm", EApiDataFormat::eRGB16_UNORM,
				"rgba16_unorm", EApiDataFormat::eRGBA16_UNORM,

				"r32_uint", EApiDataFormat::eR32_UINT,
				"rg32_uint", EApiDataFormat::eRG32_UINT,
				"rgb32_uint", EApiDataFormat::eRGB32_UINT,
				"rgba32_uint", EApiDataFormat::eRGBA32_UINT,

				"r16_sfloat", EApiDataFormat::eR16_SFLOAT,
				"rg16_sfloat", EApiDataFormat::eRG16_SFLOAT,
				"rgb16_sfloat", EApiDataFormat::eRGB16_SFLOAT,
				"rgba16_sfloat", EApiDataFormat::eRGBA16_SFLOAT,

				"r32_sfloat", EApiDataFormat::eR32_SFLOAT,
				"rg32_sfloat", EApiDataFormat::eRG32_SFLOAT,
				"rgb32_sfloat", EApiDataFormat::eRGB32_SFLOAT,
				"rgba32_sfloat", EApiDataFormat::eRGBA32_SFLOAT
			);
			
			// Register ETextureImageTypeFlagBits
			lua->new_enum(
				"TexType",
				"array", ETextureCreateFlagBits::eArray,
				"cubemap", ETextureCreateFlagBits::eCubemap,
				"one_dim", ETextureCreateFlagBits::e1D,
				"two_dim", ETextureCreateFlagBits::e2D,
				"three_dim", ETextureCreateFlagBits::e3D,
				"transferable", ETextureCreateFlagBits::eTransferable,
				"samplable", ETextureCreateFlagBits::eSamplable,
				"storage", ETextureCreateFlagBits::eStorage
			);

			// Register ETextureSizePolicy
			lua->new_enum(
				"SizePolicy",
				"full", ERenderTargetSizePolicy::eFull,
				"scalable", ERenderTargetSizePolicy::eScalable,
				"fixed", ERenderTargetSizePolicy::eFixed
			);

			// Register EQueueFamilyUsageFlagBits
			lua->new_enum(
				"QueueType",
				"present", EQueueTypeFlagBits::ePresent,
				"graphics", EQueueTypeFlagBits::eGraphics,
				"compute", EQueueTypeFlagBits::eCompute,
				"transfer", EQueueTypeFlagBits::eTransfer
			);

			// Register ERenderingType
			lua->new_enum(
				"RenderingType",
				"fullscreen", ERenderingType::eFullscreen,
				"geometry", ERenderingType::eGeometry,
				"csm", ERenderingType::eCascadedShadowMap,
				"render_script", ERenderingType::eRenderScript
			);

			// Register EComputeType
			lua->new_enum(
				"ComputeType",
				"default", EComputeType::eDefault,
				"geometry", EComputeType::eGeometry,
				"cubemap", EComputeType::eCubemap,
				"culling", EComputeType::eCulling
			);

			// Register ERenderingSizeType
			lua->new_enum(
				"RenderingSizeType",
				"from_rt", ERenderingSizeType::eFromRenderTarget,
				"from_buffer", ERenderingSizeType::eFromBuffer,
				"manual", ERenderingSizeType::eManual
			);

			lua->new_enum(
				"RenderResourceTypeFlagBits",
				"render_target", ERenderResourceTypeFlagBits::eRenderTarget,
				"storage_target", ERenderResourceTypeFlagBits::eStorageTarget,
				"static_image", ERenderResourceTypeFlagBits::eStaticImage,
				"all_image", ERenderResourceTypeFlagBits::eAllImage,
				"buffer", ERenderResourceTypeFlagBits::eBuffer
			);
		}

		void bindRenderInterfaceStructures(sol::state* lua)
		{
			// FPoint2D<> registration
			register_point2d<i8>("I8Point2D", lua);
			register_point2d<u8>("U8Point2D", lua);
			register_point2d<i16>("I16Point2D", lua);
			register_point2d<u16>("U16Point2D", lua);
			register_point2d<i32>("I32Point2D", lua);
			register_point2d<u32>("U32Point2D", lua);
			register_point2d<i64>("I64Point2D", lua);
			register_point2d<u64>("U64Point2D", lua);
			register_point2d<f32>("F32Point2D", lua);
			register_point2d<f64>("F64Point2D", lua);

			// FRect2D<> registration
			register_rect2d<i8>("I8Rect2D", lua);
			register_rect2d<u8>("U8Rect2D", lua);
			register_rect2d<i16>("I16Rect2D", lua);
			register_rect2d<u16>("U16Rect2D", lua);
			register_rect2d<i32>("I32Rect2D", lua);
			register_rect2d<u32>("U32Rect2D", lua);
			register_rect2d<i64>("I64Rect2D", lua);
			register_rect2d<u64>("U64Rect2D", lua);
			register_rect2d<f32>("F32Rect2D", lua);
			register_rect2d<f64>("F64Rect2D", lua);

			// 
			lua->new_usertype<ETextureCreateFlags>(
				"TextureCreateFlags",
				sol::constructors<ETextureCreateFlags(), ETextureCreateFlags(u32), ETextureCreateFlags(ETextureCreateFlagBits), ETextureCreateFlags(const ETextureCreateFlags&)>()
			);

			// FTextureCreateInfo registration
			lua->new_usertype<FTextureCreateInfo>(
				"TextureCI",
				sol::constructors<FTextureCreateInfo()>(),
				"format", &FTextureCreateInfo::format,
				"type", &FTextureCreateInfo::typeFlags,
				"mip_levels", &FTextureCreateInfo::mipLevels,
				"array_layers", &FTextureCreateInfo::arrayLayers,
				"set_fixed_size_3d", sol::overload(
					static_cast<void(FTextureCreateInfo::*)(u32)>(&FTextureCreateInfo::setFixedSize3D),
					static_cast<void(FTextureCreateInfo::*)(u32, u32, u32)>(&FTextureCreateInfo::setFixedSize3D)
				),
				"set_fixed_size_2d", sol::overload(
					static_cast<void(FTextureCreateInfo::*)(u32)>(&FTextureCreateInfo::setFixedSize2D),
					static_cast<void(FTextureCreateInfo::*)(u32, u32)>(&FTextureCreateInfo::setFixedSize2D)
				),
				"set_size_scale", &FTextureCreateInfo::setSizeScale,
				"enable_mip_map", &FTextureCreateInfo::enableMipMap
			);

			// FRenderTargetSizeInfo registration
			lua->new_usertype<FRenderTargetSizeInfo>(
				"RenderTargetSizeInfo",
				sol::constructors<FRenderTargetSizeInfo()>(),

				"type", &FRenderTargetSizeInfo::type,
				"target", &FRenderTargetSizeInfo::target,

				"size_x", &FRenderTargetSizeInfo::sizeX,
				"size_y", &FRenderTargetSizeInfo::sizeY,
				"size_z", &FRenderTargetSizeInfo::sizeZ
			);

			// Render graph
			// FRenderResource registration
			lua->new_usertype<FRenderResource>(
				"RenderResource",
				"type", &FRenderResource::type,
				"handle", &FRenderResource::handle
			);

			// FRenderConfigPassCreateInfo registration
			lua->new_usertype<FRenderConfigPassCreateInfo>(
				"PassCreateInfo",
				"queue", &FRenderConfigPassCreateInfo::queue,
				"shader", &FRenderConfigPassCreateInfo::shader,
				"one_time", &FRenderConfigPassCreateInfo::oneTime,
				"size_target", &FRenderConfigPassCreateInfo::sizeTarget,
				"bind_write_resource", &FRenderConfigPassCreateInfo::bindWriteResource,
				"bind_read_resource", &FRenderConfigPassCreateInfo::bindReadResource,
				"add_write_resource", &FRenderConfigPassCreateInfo::addWriteResource,
				"add_read_resource", &FRenderConfigPassCreateInfo::addReadResource
				);

			// FRenderConfigRenderCreateInfo registration
			lua->new_usertype<FRenderConfigRenderCreateInfo>(
				"GraphicsCreateInfo",
				sol::base_classes, sol::bases<FRenderConfigPassCreateInfo>(),
				"type", &FRenderConfigRenderCreateInfo::type,
				"flip_viewport", &FRenderConfigRenderCreateInfo::flipViewport,
				"set_depth_target", &FRenderConfigRenderCreateInfo::setDepthTarget,
				"set_stencil_target", &FRenderConfigRenderCreateInfo::setStencilTarget
			);

			// FRenderConfigComputeCreateInfo registration
			lua->new_usertype<FRenderConfigComputeCreateInfo>(
				"ComputeCreateInfo",
				sol::base_classes, sol::bases<FRenderConfigPassCreateInfo>(),
				"type", &FRenderConfigComputeCreateInfo::type
			);
		}

		void bindRenderInterfaceObjects(sol::state* lua)
		{
			// IRenderInterface registration
			lua->new_usertype<IRenderInterface>(
				"IRenderInterface",
				"get_shader_desc", &IRenderInterface::getShaderNameTag,
				"create_texture", &IRenderInterface::createTexture,
				"create_shader", sol::overload(
					static_cast<u64(IRenderInterface::*)(const stl::string&, const FGraphicsShaderCreateInfo&)>(&IRenderInterface::createShader),
					static_cast<u64(IRenderInterface::*)(const stl::string&, const FComputeShaderCreateInfo&)>(&IRenderInterface::createShader)
				)
			);

			// IRenderContext registration
			lua->new_usertype<IRenderContext>(
				"IRenderContext",
				"create_render_target", &IRenderContext::createRenderTarget,
				"create_shader", sol::overload(
					static_cast<u64(IRenderContext::*)(const stl::string&, const FGraphicsShaderCreateInfo&, const stl::vector<FShaderCreationBlockInfo>&)>(&IRenderContext::createShader),
					static_cast<u64(IRenderContext::*)(const stl::string&, const FComputeShaderCreateInfo&, const stl::vector<FShaderCreationBlockInfo>&)>(&IRenderContext::createShader)
				),
				"get_texture_resource", &IRenderContext::getTextureResource,
				"get_shader_resource", &IRenderContext::getShaderResource,
				"get_surface_resolution", &IRenderContext::getRenderResolution,
				"get_depth_format", &IRenderContext::getDepthFormat,
				"get_api_format", &IRenderContext::getApiFormat,
				"get_present_format", &IRenderContext::getPresentFormat,
				"get_shader_reflection", &IRenderContext::getShaderReflection,
				"get_render_rect_from_rt", &IRenderContext::getRenderRectFromRT,
				"get_screen_render_rect", &IRenderContext::getScreenRenderRect
			);

			// IRenderGraph registration
			lua->new_usertype<IRenderGraph>(
				"IRenderGraph",
				"add_static_resource", &IRenderGraph::addStaticResource,
				"add_render_resource", sol::overload(
					static_cast<u64(IRenderGraph::*)(const stl::string&, const FTextureCreateInfo&)>(&IRenderGraph::addRenderResource),
					static_cast<u64(IRenderGraph::*)(const stl::string&, const FBufferCreateInfo&)>(&IRenderGraph::addRenderResource)
				),
				"set_backbuffer_resource_name", &IRenderGraph::setBackbufferResourceName,
				"get_render_resource", sol::overload(
					static_cast<const FRenderResource&(IRenderGraph::*)(const stl::string&) const>(&IRenderGraph::getRenderResource),
					static_cast<const FRenderResource&(IRenderGraph::*)(u64) const>(&IRenderGraph::getRenderResource)
				),
				"add_render_pass", sol::overload(
					static_cast<u64(IRenderGraph::*)(const stl::string&, const FRenderConfigRenderCreateInfo&)>(&IRenderGraph::addRenderPass),
					static_cast<u64(IRenderGraph::*)(const stl::string&, const FRenderConfigComputeCreateInfo&)>(&IRenderGraph::addRenderPass)
				)
			);
		}

		void bindGlobalFunctions(sol::state* lua)
		{
			lua->set_function("calculate_mip_level_count", [](u32 width, u32 height) -> u32 {
				return utl::cast<u32>(stl::floor(stl::log2((stl::max)(width, height)))) + 1u;
				});
		}

		void bindRenderInterface(sol::state* lua)
		{
			bindRenderInterfaceEnums(lua);
			bindRenderInterfaceStructures(lua);
			bindRenderInterfaceObjects(lua);
			bindGlobalFunctions(lua);
		}
	}
}