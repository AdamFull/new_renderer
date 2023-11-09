#include "RenderConfigBindings.h"

#include <interface/RenderInterface.h>
#include <render/ShaderObject.h>
#include <sol/sol.hpp>
#include <glm/glm.hpp>

namespace engine
{
	namespace render
	{
		void bindRenderInterfaceEnums(sol::state* lua)
		{
			// Register ETextureFormat
			lua->new_enum(
				"Format",
				"rg16_sfloat", FORMAT_RG16_SFLOAT,
				"rgba8_srgb", FORMAT_RGBA8_SRGB,
				"rgba8_unorm", FORMAT_RGBA8_UNORM,
				"rgba16_sfloat", FORMAT_RGBA16_SFLOAT,
				"rgba32_sfloat", FORMAT_RGBA32_SFLOAT
			);

			// Register EResourceTypeBits
			lua->new_enum(
				"ResType",
				"texture", RESOURCE_TYPE_TEXTURE_BIT,
				"rt", RESOURCE_TYPE_RENDER_TARGET_BIT,
				"buffer", RESOURCE_TYPE_BUFFER_BIT,
				"shader", RESOURCE_TYPE_SHADER_BIT
			);

			// Register ETextureImageTypeFlagBits
			lua->new_enum(
				"TexType",
				"array", TEXTURE_IMAGE_TYPE_ARRAY_FLAG_BIT,
				"cubemap", TEXTURE_IMAGE_TYPE_CUBEMAP_FLAG_BIT,
				"one_dim", TEXTURE_IMAGE_TYPE_1D_FLAG_BIT,
				"two_dim", TEXTURE_IMAGE_TYPE_2D_FLAG_BIT,
				"three_dim", TEXTURE_IMAGE_TYPE_3D_FLAG_BIT,
				"depth", TEXTURE_IMAGE_TYPE_DEPTH_FLAG_BIT,
				"transferable", TEXTURE_IMAGE_TYPE_TRANSFERABLE_FLAG_BIT,
				"samplable", TEXTURE_IMAGE_TYPE_SAMPLABLE_FLAG_BIT,
				"storage", TEXTURE_IMAGE_TYPE_STORAGE_FLAG_BIT
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
		}

		void bindRenderInterfaceStructures(sol::state* lua)
		{
			// FResourceView registration
			lua->new_usertype<FResourceView>(
				"ResourceView",
				sol::constructors<FResourceView()>(),
				"id", &FResourceView::id,
				"mask", &FResourceView::mask
			);

			// FPoint2D<int32_t> registration
			lua->new_usertype<FPoint2D<int32_t>>(
				"I32Point2D",
				sol::constructors<FPoint2D<int32_t>()>(),
				"x", &FPoint2D<int32_t>::x,
				"y", &FPoint2D<int32_t>::y
			);

			// FPoint2D<uint32_t> registration
			lua->new_usertype<FPoint2D<uint32_t>>(
				"U32Point2D",
				sol::constructors<FPoint2D<uint32_t>()>(),
				"x", &FPoint2D<uint32_t>::x,
				"y", &FPoint2D<uint32_t>::y
			);

			// FPoint2D<float> registration
			lua->new_usertype<FPoint2D<float>>(
				"F32Point2D",
				sol::constructors<FPoint2D<float>()>(),
				"x", &FPoint2D<float>::x,
				"y", &FPoint2D<float>::y
			);

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
				"width", &FTextureCreateInfo::width,
				"height", &FTextureCreateInfo::height,
				"depth", &FTextureCreateInfo::depth,
				"mip_levels", &FTextureCreateInfo::mipLevels,
				"array_layers", &FTextureCreateInfo::arrayLayers
			);

			// FRenderingCreateInfo registration
			lua->new_usertype<FRenderingCreateInfo>(
				"RendererCI",
				sol::constructors<FRenderingCreateInfo()>(),
				"render_script", &FRenderingCreateInfo::renderScript,
				"queue", &FRenderingCreateInfo::queue,
				"shader_name", &FRenderingCreateInfo::shaderName,
				"one_time", &FRenderingCreateInfo::oneTime,
				"flip_viewport", &FRenderingCreateInfo::flipViewport,
				"input_targets", &FRenderingCreateInfo::shaderInputs,
				"output_targets", &FRenderingCreateInfo::renderTargets,
				"depth_target", &FRenderingCreateInfo::depthTarget,
				"stencil_target", &FRenderingCreateInfo::stencilTarget
			);

			// FVertexBindingDescription registration
			lua->new_usertype<FVertexBindingDescription>(
				"FVertexBindingDescription",
				sol::constructors<FVertexBindingDescription()>(),
				"binding", &FVertexBindingDescription::binding,
				"stride", &FVertexBindingDescription::stride
			);

			// FVertexAttributeDescription registration
			lua->new_usertype<FVertexAttributeDescription>(
				"VertexAttributeDescription",
				sol::constructors<FVertexDeclarationInfo()>(),
				"location", &FVertexAttributeDescription::location,
				"binding", &FVertexAttributeDescription::binding,
				"format", &FVertexAttributeDescription::format,
				"offset", &FVertexAttributeDescription::offset
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
					static_cast<FResourceView(IRenderInterface::*)(const std::string&, const FGraphicsShaderCreateInfo&)>(&IRenderInterface::createShader),
					static_cast<FResourceView(IRenderInterface::*)(const std::string&, const FComputeShaderCreateInfo&)>(&IRenderInterface::createShader)
				)
			);

			// IRenderHandleInterface registration
			lua->new_usertype<IRenderHandleInterface>(
				"IRenderHandleInterface",
				"create_render_target", &IRenderHandleInterface::createRenderTargets,
				"create_shader", sol::overload(
					static_cast<FResourceView(IRenderHandleInterface::*)(const std::string&, const FGraphicsShaderCreateInfo&, const std::vector<FShaderCreationBlockInfo>&)>(&IRenderHandleInterface::createShader),
					static_cast<FResourceView(IRenderHandleInterface::*)(const std::string&, const FComputeShaderCreateInfo&, const std::vector<FShaderCreationBlockInfo>&)>(&IRenderHandleInterface::createShader)
				),
				"get_resource", &IRenderHandleInterface::getResource,
				"get_surface_resolution", &IRenderHandleInterface::getRenderResolution,
				"get_depth_format", &IRenderHandleInterface::getDepthFormat,
				"get_api_format", &IRenderHandleInterface::getApiFormat,
				"get_present_format", &IRenderHandleInterface::getPresentFormat,
				"get_shader_reflection", &IRenderHandleInterface::getShaderReflection
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
				"new", sol::factories([]() { return std::make_unique<CShaderObject>(); }),
				"create", &IShaderObject::create,
				"get_uniform_block", [](IShaderObject& self, const std::string& name) { return self.getUniformBlock(name.c_str()).get(); },
				"bind_texture", & IShaderObject::bindTexture
			);
		}

		void bindRenderInterface(sol::state* lua)
		{
			bindRenderInterfaceEnums(lua);
			bindRenderInterfaceStructures(lua);
			bindRenderInterfaceObjects(lua);
		}
	}
}