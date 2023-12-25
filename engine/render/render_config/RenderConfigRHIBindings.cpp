#include "RenderConfigBindings.h"

#include <engine/interface/render/RenderInterface.h>
#include <engine/render/ShaderObject.h>
#include <glm/glm.hpp>

#include <sol/sol.hpp>

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
				"rgba8_srgb", FORMAT_RGBA8_SRGB,

				"r8_unorm", FORMAT_R8_UNORM,
				"rg8_unorm", FORMAT_RG8_UNORM,
				"rgb8_unorm", FORMAT_RGB8_UNORM,
				"rgba8_unorm", FORMAT_RGBA8_UNORM,

				"r16_unorm", FORMAT_R16_UNORM,
				"rg16_unorm", FORMAT_RG16_UNORM,
				"rgb16_unorm", FORMAT_RGB16_UNORM,
				"rgba16_unorm", FORMAT_RGBA16_UNORM,

				"r32_uint", FORMAT_R32_UINT,
				"rg32_uint", FORMAT_RG32_UINT,
				"rgb32_uint", FORMAT_RGB32_UINT,
				"rgba32_uint", FORMAT_RGBA32_UINT,

				"r16_sfloat", FORMAT_R16_SFLOAT,
				"rg16_sfloat", FORMAT_RG16_SFLOAT,
				"rgb16_sfloat", FORMAT_RGB16_SFLOAT,
				"rgba16_sfloat", FORMAT_RGBA16_SFLOAT,

				"r32_sfloat", FORMAT_R32_SFLOAT,
				"rg32_sfloat", FORMAT_RG32_SFLOAT,
				"rgb32_sfloat", FORMAT_RGB32_SFLOAT,
				"rgba32_sfloat", FORMAT_RGBA32_SFLOAT
			);

			// Register ETextureImageTypeFlagBits
			lua->new_enum(
				"TexType",
				"array", TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT,
				"cubemap", TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT,
				"one_dim", TEXTURE_IMAGE_TYPE_1D_FLAG_BIT,
				"two_dim", TEXTURE_IMAGE_TYPE_2D_FLAG_BIT,
				"three_dim", TEXTURE_IMAGE_TYPE_3D_FLAG_BIT,
				"transferable", TEXTURE_IMAGE_TYPE_TRANSFERABLE_FLAG_BIT,
				"samplable", TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT,
				"storage", TEXTURE_IMAGE_TYPE_STORAGE_FLAG_BIT
			);

			// Register ETextureSizePolicy
			lua->new_enum(
				"SizePolicy",
				"full", TEXTURE_SIZE_POLICY_FULL,
				"scalable", TEXTURE_SIZE_POLICY_SCALABLE,
				"fixed", TEXTURE_SIZE_POLICY_FIXED
			);

			// Register EQueueFamilyUsageFlagBits
			lua->new_enum(
				"QueueType",
				"present", QUEUE_FAMILY_USAGE_PRESENT_FLAG_BIT,
				"graphics", QUEUE_FAMILY_USAGE_GRAPHICS_FLAG_BIT,
				"compute", QUEUE_FAMILY_USAGE_COMPUTE_FLAG_BIT,
				"transfer", QUEUE_FAMILY_USAGE_TRANSFER_FLAG_BIT
			);

			// Register EShaderPrimitiveTopology
			lua->new_enum(
				"PrimitiveTopology",
				"point_list", SHADER_PRIMITIVE_TOPOLOGY_POINT_LIST,
				"line_list", SHADER_PRIMITIVE_TOPOLOGY_LINE_LIST,
				"line_strip", SHADER_PRIMITIVE_TOPOLOGY_LINE_STRIP,
				"triangle_list", SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				"triangle_strip", SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
				"triangle_fan", SHADER_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN
			);

			// Register EShaderCullMode
			lua->new_enum(
				"CullMode",
				"none", SHADER_CULL_MODE_NONE,
				"front", SHADER_CULL_MODE_FRONT_BIT,
				"back", SHADER_CULL_MODE_BACK_BIT,
				"double_sided", SHADER_CULL_MODE_FRONT_AND_BACK
			);

			// Register EShaderFrontFace
			lua->new_enum(
				"FrontFace",
				"ccw", SHADER_FRONT_FACE_CCW,
				"cw", SHADER_FRONT_FACE_CW
			);

			// Register EShaderResourcesUpdateFlagBits
			lua->new_enum(
				"ShaderUpdateFlags",
				"uniform_blocks", SHADER_RESOURCES_UPDATE_UNIFORM_BLOCKS_FLAG_BIT,
				"textures", SHADER_RESOURCES_UPDATE_TEXTURES_FLAG_BIT,
				"all", SHADER_RESOURCES_UPDATE_ALL_FLAG_BIT
			);

			// Register ERenderingStepType
			lua->new_enum(
				"RenderingStepType",
				"synchronization", RENDERING_STEP_SYNCHRONIZATION_TYPE,
				"rendering", RENDERING_STEP_RENDERING_TYPE,
				"compute", RENDERING_STEP_COMPUTE_TYPE
			);

			// Register ERenderingType
			lua->new_enum(
				"RenderingType",
				"fullscreen", RENDERING_FULLSCREEN_TYPE,
				"geometry", RENDERING_GEOMETRY_TYPE,
				"csm", RENDERING_CSM_TYPE,
				"render_script", RENDERING_RENDER_SCRIPT_TYPE
			);

			// Register EComputeType
			lua->new_enum(
				"ComputeType",
				"default", COMPUTE_DEFAULT_TYPE,
				"geometry", COMPUTE_GEOMETRY_TYPE,
				"cubemap", COMPUTE_CUBEMAP_TYPE
			);

			// Register ESynchronizationType
			lua->new_enum(
				"SyncType",
				"semaphore", SYNCHRONIZATION_SEMAPHORE_TYPE,
				"barrier", SYNCHRONIZATION_BARRIER_TYPE
			);

			// Register ESemaphoreMode
			lua->new_enum(
				"SemaphoreMode",
				"signal", SEMAPHORE_SIGNAL_MODE,
				"wait", SEMAPHORE_WAIT_MODE
			);

			// Register EBarrierType
			lua->new_enum(
				"BarrierType",
				"g2c", BARRIER_FROM_GRAPHICS_TO_COMPUTE,
				"g2g", BARRIER_FROM_GRAPHICS_TO_GRAPHICS,
				"c2c", BARRIER_FROM_COMPUTE_TO_COMPUTE,
				"c2g", BARRIER_FROM_COMPUTE_TO_GRAPHICS
			);
		}

		void bindRenderInterfaceStructures(sol::state* lua)
		{
			// FPoint2D<> registration
			register_point2d<int8_t>("I8Point2D", lua);
			register_point2d<uint8_t>("U8Point2D", lua);
			register_point2d<int16_t>("I16Point2D", lua);
			register_point2d<uint16_t>("U16Point2D", lua);
			register_point2d<int32_t>("I32Point2D", lua);
			register_point2d<uint32_t>("U32Point2D", lua);
			register_point2d<int64_t>("I64Point2D", lua);
			register_point2d<uint64_t>("U64Point2D", lua);
			register_point2d<float>("F32Point2D", lua);
			register_point2d<double>("F64Point2D", lua);

			// FRect2D<> registration
			register_rect2d<int8_t>("I8Rect2D", lua);
			register_rect2d<uint8_t>("U8Rect2D", lua);
			register_rect2d<int16_t>("I16Rect2D", lua);
			register_rect2d<uint16_t>("U16Rect2D", lua);
			register_rect2d<int32_t>("I32Rect2D", lua);
			register_rect2d<uint32_t>("U32Rect2D", lua);
			register_rect2d<int64_t>("I64Rect2D", lua);
			register_rect2d<uint64_t>("U64Rect2D", lua);
			register_rect2d<float>("F32Rect2D", lua);
			register_rect2d<double>("F64Rect2D", lua);

			// FShaderUploadInfo registration
			lua->new_usertype<FShaderUploadInfo>(
				"ShaderNameDesc",
				sol::constructors<FShaderUploadInfo()>(),
				"library", &FShaderUploadInfo::libraryId,
				"shader", &FShaderUploadInfo::shaderId,
				"name", &FShaderUploadInfo::originalName
			);

			// FTextureCreateInfo registration
			lua->new_usertype<FTextureCreateInfo>(
				"TextureCI",
				sol::constructors<FTextureCreateInfo()>(),
				"present_image", &FTextureCreateInfo::isSwapchainTarget,
				"format", &FTextureCreateInfo::format,
				"type", &FTextureCreateInfo::typeFlags,
				"size_policy", &FTextureCreateInfo::sizePolicy,
				"width", &FTextureCreateInfo::width,
				"height", &FTextureCreateInfo::height,
				"depth", &FTextureCreateInfo::depth,
				"mip_levels", &FTextureCreateInfo::mipLevels,
				"array_layers", &FTextureCreateInfo::arrayLayers
			);

			// FRenderingStepSemaphore registration
			lua->new_usertype<FRenderingStepSemaphore>(
				"SemaphoreStep",
				sol::constructors<FRenderingStepSemaphore()>(),
				"type", &FRenderingStepSemaphore::type,
				"value", &FRenderingStepSemaphore::value,
				"srcQueue", &FRenderingStepSemaphore::srcQueue,
				"dstQueue", &FRenderingStepSemaphore::dstQueue
			);

			// FRenderingStepBarrier registration
			lua->new_usertype<FRenderingStepBarrier>(
				"BarrierStep",
				sol::constructors<FRenderingStepBarrier()>(),
				"type", &FRenderingStepBarrier::type,
				"queue", &FRenderingStepBarrier::queue,
				"targets", &FRenderingStepBarrier::targets
			);

			// FRenderingStepSynchronizationInfo registration
			lua->new_usertype<FRenderingStepSynchronizationInfo>(
				"SynchronizationStepInfo",
				sol::constructors<FRenderingStepSynchronizationInfo()>(),
				"type", &FRenderingStepSynchronizationInfo::type,
				"semaphore", &FRenderingStepSynchronizationInfo::semaphore,
				"barrier", &FRenderingStepSynchronizationInfo::barrier
			);

			// FRenderingStepRenderingInfo registration
			lua->new_usertype<FRenderingStepRenderingInfo>(
				"RenderingStepInfo",
				sol::constructors<FRenderingStepRenderingInfo()>(),
				"type", &FRenderingStepRenderingInfo::type,
				"queue", &FRenderingStepRenderingInfo::queue,

				"render_script", &FRenderingStepRenderingInfo::renderScript,
				"shader_name", &FRenderingStepRenderingInfo::shaderName,

				"one_time", & FRenderingStepRenderingInfo::oneTime,
				"flip_viewport", & FRenderingStepRenderingInfo::flipViewport,
				"material_pass", & FRenderingStepRenderingInfo::materialPass,

				"render_rect", & FRenderingStepRenderingInfo::renderRect,

				"input_targets", & FRenderingStepRenderingInfo::shaderInputs,
				"output_targets", & FRenderingStepRenderingInfo::renderTargets,
				"depth_target", & FRenderingStepRenderingInfo::depthTarget,
				"stencil_target", & FRenderingStepRenderingInfo::stencilTarget
			);

			// FRenderingStepComputeInfo registration
			lua->new_usertype<FRenderingStepComputeInfo>(
				"ComputeStepInfo",
				sol::constructors<FRenderingStepComputeInfo()>(),
				"type", &FRenderingStepComputeInfo::type,
				"queue", &FRenderingStepComputeInfo::queue,

				"render_script", & FRenderingStepComputeInfo::renderScript,
				"shader_name", & FRenderingStepComputeInfo::shaderName,

				"one_time", & FRenderingStepComputeInfo::oneTime,

				"input_targets", & FRenderingStepComputeInfo::shaderInputs,
				"write_targets", & FRenderingStepComputeInfo::writeTargets,
				"write_buffers", & FRenderingStepComputeInfo::writeBuffers,

				"group_x", & FRenderingStepComputeInfo::groupX,
				"group_y", & FRenderingStepComputeInfo::groupY,
				"group_z", & FRenderingStepComputeInfo::groupZ
			);

			// FRenderingStepCreateInfo registration
			lua->new_usertype<FRenderingStepCreateInfo>(
				"RenderingStepCI",
				sol::constructors<FRenderingStepCreateInfo()>(),
				"type", &FRenderingStepCreateInfo::type,
				"sync", &FRenderingStepCreateInfo::synchronization,
				"rendering", &FRenderingStepCreateInfo::rendering,
				"compute", &FRenderingStepCreateInfo::compute
			);

			// FVertexBindingDescription registration
			lua->new_usertype<FVertexDeclarationBindingDescription>(
				"VertexBindingDescription",
				sol::constructors<FVertexDeclarationBindingDescription()>(),
				"binding", &FVertexDeclarationBindingDescription::binding,
				"stride", &FVertexDeclarationBindingDescription::stride
			);

			// FVertexAttributeDescription registration
			lua->new_usertype<FVertexDeclarationAttributeDescription>(
				"VertexAttributeDescription",
				sol::constructors<FVertexDeclarationAttributeDescription()>(),
				"location", &FVertexDeclarationAttributeDescription::location,
				"binding", &FVertexDeclarationAttributeDescription::binding,
				"format", &FVertexDeclarationAttributeDescription::format,
				"offset", &FVertexDeclarationAttributeDescription::offset
			);

			// FVertexDeclaration registration
			lua->new_usertype<FVertexDeclarationInfo>(
				"VertexDeclaration",
				sol::constructors<FVertexDeclarationInfo()>(),
				"binding_descriptions", &FVertexDeclarationInfo::bindingDescriptions,
				"attribute_descriptions", &FVertexDeclarationInfo::attributeDescriptions
			);

			// FGraphicsShaderCreateInfo registration
			lua->new_usertype<FGraphicsShaderCreateInfo>(
				"GraphicsShaderCI",
				sol::constructors<FGraphicsShaderCreateInfo()>(),
				"primitive_topology", &FGraphicsShaderCreateInfo::primitiveTopology,
				"cull_mode", &FGraphicsShaderCreateInfo::cullMode,
				"front_face", &FGraphicsShaderCreateInfo::frontFace,
				"usage_count", &FGraphicsShaderCreateInfo::usageCount,
				"input_attachments", &FGraphicsShaderCreateInfo::colorAttachments,
				"depth_target", &FGraphicsShaderCreateInfo::depthAttachment,
				"stencil_target", &FGraphicsShaderCreateInfo::stencilAttachment,
				"vertex_declaration", &FGraphicsShaderCreateInfo::vertexDeclaration
			);

			// FComputeShaderCreateInfo registration
			lua->new_usertype<FComputeShaderCreateInfo>(
				"ComputeShaderCI",
				sol::constructors<FComputeShaderCreateInfo()>(),
				"usage_count", &FComputeShaderCreateInfo::usageCount
			);

			// FReflectionUniform registration
			lua->new_usertype<FReflectionUniform>(
				"ReflectionUniform",
				sol::constructors<FReflectionUniform()>(),
				"set", &FReflectionUniform::set,
				"binding", &FReflectionUniform::binding,
				"offset", &FReflectionUniform::offset,
				"size", &FReflectionUniform::size
			);

			// FReflectionUniformBlock registration
			lua->new_usertype<FReflectionUniformBlock<FReflectionUniform>>(
				"ReflectionUniformBlock",
				sol::constructors<FReflectionUniformBlock<FReflectionUniform>()>(),
				"set", &FReflectionUniformBlock<FReflectionUniform>::set,
				"binding", &FReflectionUniformBlock<FReflectionUniform>::binding,
				"size", &FReflectionUniformBlock<FReflectionUniform>::size,
				"uniforms", &FReflectionUniformBlock<FReflectionUniform>::uniforms
			);

			// FShaderCreationBlockInfo registration
			lua->new_usertype<FShaderCreationBlockInfo>(
				"ShaderCreationBlockInfo",
				sol::constructors<FShaderCreationBlockInfo()>(),
				"stage", &FShaderCreationBlockInfo::stage,
				"code", &FShaderCreationBlockInfo::code
			);

			// FReflectionPushConstantBlock registration
			lua->new_usertype<FReflectionPushConstantBlock<FReflectionUniform>>(
				"ReflectionPushConstantBlock",
				sol::constructors<FReflectionPushConstantBlock<FReflectionUniform>()>(),
				"size", &FReflectionPushConstantBlock<FReflectionUniform>::size,
				"uniforms", &FReflectionPushConstantBlock<FReflectionUniform>::uniforms
			);

			// FShaderReflectionData registration
			lua->new_usertype<FShaderReflectionData>(
				"ShaderReflectionData",
				sol::constructors<FShaderReflectionData()>(),
				"uniforms", &FShaderReflectionData::uniforms,
				"uniform_blocks", &FShaderReflectionData::uniformBlocks,
				"push_blocks", &FShaderReflectionData::pushBlocks
			);

			// FBeginRenderingInfo registration
			lua->new_usertype<FBeginRenderingInfo>(
				"BeginRenderingInfo",
				"queue", &FBeginRenderingInfo::queue,
				"render_area", &FBeginRenderingInfo::renderArea,
				"flip_viewport", &FBeginRenderingInfo::flipViewport,
				"color_attachments", &FBeginRenderingInfo::colorAttachments,
				"depth_attachment", &FBeginRenderingInfo::depthAttachment,
				"stencil_attachment", &FBeginRenderingInfo::stencilAttachment
			);
		}

		void bindRenderInterfaceObjects(sol::state* lua)
		{
			// IRenderInterface registration
			lua->new_usertype<IRenderInterface>(
				"IRenderInterface",
				"get_shader_desc", &IRenderInterface::getShaderNameTag,
				"create_renderer", &IRenderInterface::createRendering,
				"create_texture", &IRenderInterface::createTexture,
				"create_shader", sol::overload(
					static_cast<size_t(IRenderInterface::*)(const std::string&, const FGraphicsShaderCreateInfo&)>(&IRenderInterface::createShader),
					static_cast<size_t(IRenderInterface::*)(const std::string&, const FComputeShaderCreateInfo&)>(&IRenderInterface::createShader)
				)
			);

			// IRenderContext registration
			lua->new_usertype<IRenderContext>(
				"IRenderContext",
				"create_render_target", &IRenderContext::createRenderTargets,
				"create_shader", sol::overload(
					static_cast<size_t(IRenderContext::*)(const std::string&, const FGraphicsShaderCreateInfo&, const std::vector<FShaderCreationBlockInfo>&)>(&IRenderContext::createShader),
					static_cast<size_t(IRenderContext::*)(const std::string&, const FComputeShaderCreateInfo&, const std::vector<FShaderCreationBlockInfo>&)>(&IRenderContext::createShader)
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

			// Uniform block handler registration
			lua->new_usertype<CUniformHandle>(
				"UniformHandle",
				"create", sol::overload(
					&CUniformHandle::create<FReflectionUniformBlock<FReflectionUniform>>,
					&CUniformHandle::create<FReflectionPushConstantBlock<FReflectionUniform>>
				),
				"set", sol::overload(
					&CUniformHandle::set<int8_t>,
					&CUniformHandle::set<uint8_t>,
					&CUniformHandle::set<int16_t>,
					&CUniformHandle::set<uint16_t>,
					&CUniformHandle::set<int32_t>,
					&CUniformHandle::set<uint32_t>,
					&CUniformHandle::set<int64_t>,
					&CUniformHandle::set<uint64_t>,
					&CUniformHandle::set<uint64_t>,
					&CUniformHandle::set<float>,
					&CUniformHandle::set<double>,
					&CUniformHandle::set<bool>,
					&CUniformHandle::set<glm::vec2>,
					&CUniformHandle::set<glm::vec3>,
					&CUniformHandle::set<glm::vec4>,
					&CUniformHandle::set<glm::dvec2>,
					&CUniformHandle::set<glm::dvec3>,
					&CUniformHandle::set<glm::dvec4>,
					&CUniformHandle::set<glm::ivec2>,
					&CUniformHandle::set<glm::ivec3>,
					&CUniformHandle::set<glm::ivec4>,
					&CUniformHandle::set<glm::uvec2>,
					&CUniformHandle::set<glm::uvec3>,
					&CUniformHandle::set<glm::uvec4>,
					&CUniformHandle::set<glm::bvec2>,
					&CUniformHandle::set<glm::bvec3>,
					&CUniformHandle::set<glm::bvec4>,
					&CUniformHandle::set<glm::mat2>,
					&CUniformHandle::set<glm::mat3>,
					&CUniformHandle::set<glm::mat4>,
					&CUniformHandle::set<glm::dmat2>,
					&CUniformHandle::set<glm::dmat3>,
					&CUniformHandle::set<glm::dmat4>
				)
			);

			// Shader object registration
			lua->new_usertype<IShaderObject>(
				"ShaderObject",
				//"new", sol::factories([]() { return std::make_unique<CShaderObject>(); }),
				"create", sol::overload(
					static_cast<void(IShaderObject::*)(const std::string&, const FGraphicsShaderCreateInfo&)>(&IShaderObject::create),
					static_cast<void(IShaderObject::*)(const std::string&, const FComputeShaderCreateInfo&)>(&IShaderObject::create)
				),
				"get_uniform_block", [](IShaderObject& self, const std::string& name) { return self.getUniformBlock(name.c_str()).get(); },
				"bind_texture", & IShaderObject::bindTexture
			);
		}

		void bindGlobalFunctions(sol::state* lua)
		{
			lua->set_function("calculate_mip_level_count", [](uint32_t width, uint32_t height) -> uint32_t {
				return static_cast<uint32_t>(std::floor(std::log2((std::max)(width, height)))) + 1u;
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