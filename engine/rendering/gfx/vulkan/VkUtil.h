#pragma once

#include <Structures.h>
#include <vulkan/vulkan.hpp>

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			inline vk::ShaderStageFlags ShaderStageFlagsToVk(ShaderStageFlags flags)
			{
				vk::ShaderStageFlags stage_flags{};

				if (flags & ShaderStageFlag::eVertex)
					stage_flags |= vk::ShaderStageFlagBits::eVertex;
				if (flags & ShaderStageFlag::eTessellationControl)
					stage_flags |= vk::ShaderStageFlagBits::eTessellationControl;
				if (flags & ShaderStageFlag::eTessellationEvaluation)
					stage_flags |= vk::ShaderStageFlagBits::eTessellationEvaluation;
				if (flags & ShaderStageFlag::eGeometry)
					stage_flags |= vk::ShaderStageFlagBits::eGeometry;
				if (flags & ShaderStageFlag::eFragment)
					stage_flags |= vk::ShaderStageFlagBits::eFragment;
				if (flags & ShaderStageFlag::eCompute)
					stage_flags |= vk::ShaderStageFlagBits::eCompute;

				return stage_flags;
			}

			inline vk::ShaderStageFlagBits ShaderStageFlagToVk(ShaderStageFlag flag)
			{
				if (flag == ShaderStageFlag::eVertex)
					return vk::ShaderStageFlagBits::eVertex;
				if (flag == ShaderStageFlag::eTessellationControl)
					return vk::ShaderStageFlagBits::eTessellationControl;
				if (flag == ShaderStageFlag::eTessellationEvaluation)
					return vk::ShaderStageFlagBits::eTessellationEvaluation;
				if (flag == ShaderStageFlag::eGeometry)
					return vk::ShaderStageFlagBits::eGeometry;
				if (flag == ShaderStageFlag::eFragment)
					return vk::ShaderStageFlagBits::eFragment;
				if (flag == ShaderStageFlag::eCompute)
					return vk::ShaderStageFlagBits::eCompute;

				return vk::ShaderStageFlagBits::eAll;
			}

			inline vk::PipelineStageFlags2KHR StageFlagsToVk(StageFlags flags)
			{
				vk::PipelineStageFlags2KHR vk_flags{};

				if (flags & StageFlag::eNone)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eNone;
				if (flags & StageFlag::eTopOfPipe)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eTopOfPipe;
				if (flags & StageFlag::eDrawIndirect)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eDrawIndirect;
				if (flags & StageFlag::eVertexInput)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eVertexInput;
				if (flags & StageFlag::eTessellationControlShader)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eTessellationControlShader;
				if (flags & StageFlag::eTessellationEvaluationShader)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eTessellationEvaluationShader;
				if (flags & StageFlag::eGeometryShader)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eGeometryShader;
				if (flags & StageFlag::eFragmentShader)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eFragmentShader;
				if (flags & StageFlag::eEarlyFragmentTests)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eEarlyFragmentTests;
				if (flags & StageFlag::eLateFragmentTests)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eLateFragmentTests;
				if (flags & StageFlag::eColorAttachmentOutput)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eColorAttachmentOutput;
				if (flags & StageFlag::eComputeShader)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eComputeShader;
				if (flags & StageFlag::eAllTransfer)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eAllTransfer;
				if (flags & StageFlag::eTransfer)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eTransfer;
				if (flags & StageFlag::eBottomOfPipe)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eBottomOfPipe;
				if (flags & StageFlag::eHost)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eHost;
				if (flags & StageFlag::eAllGraphics)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eAllGraphics;
				if (flags & StageFlag::eAllCommands)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eAllCommands;
				if (flags & StageFlag::eCopy)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eCopy;
				if (flags & StageFlag::eResolve)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eResolve;
				if (flags & StageFlag::eBlit)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eBlit;
				if (flags & StageFlag::eClear)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eClear;
				if (flags & StageFlag::eIndexInput)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eIndexInput;
				if (flags & StageFlag::eVertexAttributeInput)
					vk_flags |= vk::PipelineStageFlagBits2KHR::eVertexAttributeInput;
				if (flags & StageFlag::ePreRasterizationShaders)
					vk_flags |= vk::PipelineStageFlagBits2KHR::ePreRasterizationShaders;

				return vk_flags;
			}

			inline vk::ImageLayout ResourceStateToVkImageLayout(ResourceStateFlags flags)
			{
				if (flags & ResourceStateFlag::eUndefined)
					return vk::ImageLayout::eUndefined;
				else if ((flags & ResourceStateFlag::eColorAttachmentRead) || (flags & ResourceStateFlag::eColorAttachmentWrite))
					return vk::ImageLayout::eColorAttachmentOptimal;
				else if ((flags & ResourceStateFlag::eDepthAttachmentRead) || (flags & ResourceStateFlag::eStencilAttachmentRead))
					return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
				else if ((flags & ResourceStateFlag::eDepthAttachmentWrite) || (flags & ResourceStateFlag::eStencilAttachmentWrite))
					return vk::ImageLayout::eDepthStencilAttachmentOptimal;
				else if (flags & ResourceStateFlag::eDepthAttachmentRead)
					return vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal;
				else if (flags & ResourceStateFlag::eDepthAttachmentWrite)
					return vk::ImageLayout::eDepthAttachmentOptimal;
				else if (flags & ResourceStateFlag::eStencilAttachmentRead)
					return vk::ImageLayout::eStencilReadOnlyOptimal;
				else if (flags & ResourceStateFlag::eStencilAttachmentWrite)
					return vk::ImageLayout::eStencilAttachmentOptimal;
				else if (flags & ResourceStateFlag::eShaderRead)
					return vk::ImageLayout::eShaderReadOnlyOptimal;
				else if (flags & ResourceStateFlag::eShaderWrite)
					return vk::ImageLayout::eGeneral;
				else if (flags & ResourceStateFlag::eTranfserSource)
					return vk::ImageLayout::eTransferSrcOptimal;
				else if (flags & ResourceStateFlag::eTransferDestination)
					return vk::ImageLayout::eTransferDstOptimal;
				else if (flags & ResourceStateFlag::ePresent)
					return vk::ImageLayout::ePresentSrcKHR;

				assert(false && "UNSUPPORTED STATE");
				return vk::ImageLayout::eUndefined;
			}

			inline vk::AccessFlags2KHR ResourceStateToVkAccessFlags(ResourceStateFlags flags)
			{
				vk::AccessFlags2KHR access{};

				if ((flags & ResourceStateFlag::eUndefined) || (flags & ResourceStateFlag::ePresent))
					access |= vk::AccessFlagBits2KHR::eNone;
				if (flags & ResourceStateFlag::eColorAttachmentRead)
					access |= vk::AccessFlagBits2KHR::eColorAttachmentRead;
				if (flags & ResourceStateFlag::eColorAttachmentWrite)
					access |= vk::AccessFlagBits2KHR::eColorAttachmentWrite;
				if ((flags & ResourceStateFlag::eDepthAttachmentRead) || (flags & ResourceStateFlag::eStencilAttachmentRead))
					access |= vk::AccessFlagBits2KHR::eDepthStencilAttachmentRead;
				if ((flags & ResourceStateFlag::eDepthAttachmentWrite) || (flags & ResourceStateFlag::eStencilAttachmentWrite))
					access |= vk::AccessFlagBits2KHR::eDepthStencilAttachmentWrite;
				if (flags & ResourceStateFlag::eShaderRead)
					access |= vk::AccessFlagBits2KHR::eShaderRead | vk::AccessFlagBits2KHR::eShaderSampledRead | vk::AccessFlagBits2KHR::eShaderStorageRead;
				if (flags & ResourceStateFlag::eShaderWrite)
					access |= vk::AccessFlagBits2KHR::eShaderWrite | vk::AccessFlagBits2KHR::eShaderStorageWrite;
				if (flags & ResourceStateFlag::eTranfserSource)
					access |= vk::AccessFlagBits2KHR::eTransferRead;
				if (flags & ResourceStateFlag::eTransferDestination)
					access |= vk::AccessFlagBits2KHR::eTransferWrite;
				if (flags & ResourceStateFlag::eIndirect)
					access |= vk::AccessFlagBits2KHR::eIndirectCommandRead;

				return access;
			}

			inline vk::PipelineStageFlags2KHR ResourceStateToVkStageFlags(ResourceStateFlags flags)
			{
				vk::PipelineStageFlags2KHR stages{};

				if ((flags & ResourceStateFlag::eUndefined) || (flags & ResourceStateFlag::ePresent))
					stages |= vk::PipelineStageFlagBits2KHR::eAllCommands;
				if (flags & ResourceStateFlag::eColorAttachmentWrite)
					stages |= vk::PipelineStageFlagBits2KHR::eColorAttachmentOutput;
				if ((flags & ResourceStateFlag::eDepthAttachmentRead) || (flags & ResourceStateFlag::eDepthAttachmentWrite))
					stages |= vk::PipelineStageFlagBits2KHR::eEarlyFragmentTests | vk::PipelineStageFlagBits2KHR::eLateFragmentTests;
				if ((flags & ResourceStateFlag::eStencilAttachmentRead) || flags & ResourceStateFlag::eStencilAttachmentWrite)
					stages |= vk::PipelineStageFlagBits2KHR::eEarlyFragmentTests | vk::PipelineStageFlagBits2KHR::eLateFragmentTests;
				if (flags & ResourceStateFlag::eShaderRead)
					stages |= vk::PipelineStageFlagBits2KHR::eAllGraphics | vk::PipelineStageFlagBits2KHR::eComputeShader;
				if (flags & ResourceStateFlag::eShaderWrite)
					stages |= vk::PipelineStageFlagBits2KHR::eComputeShader;
				if ((flags & ResourceStateFlag::eTranfserSource) || (flags & ResourceStateFlag::eTransferDestination))
					stages |= vk::PipelineStageFlagBits2KHR::eTransfer | vk::PipelineStageFlagBits2KHR::eCopy;
				if (flags & ResourceStateFlag::eIndirect)
					stages |= vk::PipelineStageFlagBits2KHR::eDrawIndirect;

				return stages;
			}

			inline vk::DescriptorType DescriptorTypeToVk(DescriptorType type)
			{
				if (type == DescriptorType::eSampler)
					return vk::DescriptorType::eSampler;
				else if (type == DescriptorType::eCombinedImageSampler)
					return vk::DescriptorType::eCombinedImageSampler;
				else if (type == DescriptorType::eSampledImage)
					return vk::DescriptorType::eSampledImage;
				else if (type == DescriptorType::eStorageImage)
					return vk::DescriptorType::eStorageImage;
				else if (type == DescriptorType::eUniformTexelBuffer)
					return vk::DescriptorType::eUniformTexelBuffer;
				else if (type == DescriptorType::eStorageTexelBuffer)
					return vk::DescriptorType::eStorageTexelBuffer;
				else if (type == DescriptorType::eUniformBuffer)
					return vk::DescriptorType::eUniformBuffer;
				else if (type == DescriptorType::eStorageBuffer)
					return vk::DescriptorType::eStorageBuffer;
				else if (type == DescriptorType::eUniformBufferDynamic)
					return vk::DescriptorType::eUniformBufferDynamic;
				else if (type == DescriptorType::eStorageBufferDynamic)
					return vk::DescriptorType::eStorageBufferDynamic;
				else if (type == DescriptorType::eInputAttachment)
					return vk::DescriptorType::eInputAttachment;

				assert(false && "NOT IMPLEMENTED STATE");
				return vk::DescriptorType::eSampler;
			}

			inline vk::Filter FilterToVk(Filter filter)
			{
				if (filter == Filter::eNearest)
					return vk::Filter::eNearest;
				else if (filter == Filter::eLinear)
					return vk::Filter::eLinear;

				assert(false && "NOT IMPLEMENTED STATE");
				return vk::Filter::eNearest;
			}

			inline vk::SamplerMipmapMode MipmapModeToVk(Filter mode)
			{
				if (mode == Filter::eNearest)
					return vk::SamplerMipmapMode::eNearest;
				else if (mode == Filter::eLinear)
					return vk::SamplerMipmapMode::eLinear;

				assert(false && "NOT IMPLEMENTED STATE");
				return vk::SamplerMipmapMode::eNearest;
			}

			inline vk::SamplerAddressMode AddressModeToVk(SamplerAddressMode mode)
			{
				if (mode == SamplerAddressMode::eMirror)
					return vk::SamplerAddressMode::eMirroredRepeat;
				else if (mode == SamplerAddressMode::eRepeat)
					return vk::SamplerAddressMode::eRepeat;
				else if (mode == SamplerAddressMode::eClampToEdge)
					return vk::SamplerAddressMode::eClampToEdge;
				else if (mode == SamplerAddressMode::eClampToBorder)
					return vk::SamplerAddressMode::eClampToBorder;

				assert(false && "NOT IMPLEMENTED STATE");
				return vk::SamplerAddressMode::eRepeat;
			}

			inline vk::CompareOp CompareOpToVk(CompareOp op)
			{
				if (op == CompareOp::eNever)
					return vk::CompareOp::eNever;
				else if (op == CompareOp::eLess)
					return vk::CompareOp::eLess;
				else if (op == CompareOp::eEqual)
					return vk::CompareOp::eEqual;
				else if (op == CompareOp::eLessOrEqual)
					return vk::CompareOp::eLessOrEqual;
				else if (op == CompareOp::eGreater)
					return vk::CompareOp::eGreater;
				else if (op == CompareOp::eNotEqual)
					return vk::CompareOp::eNotEqual;
				else if (op == CompareOp::eGreaterOrEqual)
					return vk::CompareOp::eGreaterOrEqual;
				else if (op == CompareOp::eAlways)
					return vk::CompareOp::eAlways;

				//assert(false && "NOT IMPLEMENTED STATE");
				return vk::CompareOp::eNever;
			}

			inline vk::QueryType QueryTypeToVk(QueryType type)
			{
				if (type == QueryType::eTimestamp)
					return vk::QueryType::eTimestamp;
				else if (type == QueryType::eOcclusion)
					return vk::QueryType::eOcclusion;
				else if (type == QueryType::ePipelineStatistics)
					return vk::QueryType::ePipelineStatistics;

				assert(false && "NOT IMPLEMENTED STATE");
				return vk::QueryType::eOcclusion;
			}

			inline vk::PipelineBindPoint PipelineTypeToVk(PipelineType type)
			{
				if (type == PipelineType::eCompute)
					return vk::PipelineBindPoint::eCompute;
				else if (type == PipelineType::eGraphics)
					return vk::PipelineBindPoint::eGraphics;

				assert(false && "NOT IMPLEMENTED STATE");
				return vk::PipelineBindPoint::eGraphics;
			}

			inline vk::PrimitiveTopology PrimitiveTopologyToVk(PrimitiveTopology topology)
			{
				if (topology == PrimitiveTopology::ePointList)
					return vk::PrimitiveTopology::ePointList;
				else if (topology == PrimitiveTopology::eLineList)
					return vk::PrimitiveTopology::eLineList;
				else if (topology == PrimitiveTopology::eLineStrip)
					return vk::PrimitiveTopology::eLineStrip;
				else if (topology == PrimitiveTopology::eTriangleList)
					return vk::PrimitiveTopology::eTriangleList;
				else if (topology == PrimitiveTopology::eTriangleStrip)
					return vk::PrimitiveTopology::eTriangleStrip;
				else if (topology == PrimitiveTopology::ePatchList)
					return vk::PrimitiveTopology::ePatchList;

				assert(false && "NOT IMPLEMENTED STATE");
				return vk::PrimitiveTopology::eTriangleList;
			}

			inline vk::SampleCountFlagBits SampleCountToVk(SampleCount sample_count)
			{
				if (sample_count == SampleCount::e1)
					return vk::SampleCountFlagBits::e1;
				else if (sample_count == SampleCount::e2)
					return vk::SampleCountFlagBits::e2;
				else if (sample_count == SampleCount::e4)
					return vk::SampleCountFlagBits::e4;
				else if (sample_count == SampleCount::e8)
					return vk::SampleCountFlagBits::e8;
				else if (sample_count == SampleCount::e16)
					return vk::SampleCountFlagBits::e16;

				return vk::SampleCountFlagBits::e1;
			}

			inline vk::PolygonMode FillModeToVk(FillMode mode)
			{
				if (mode == FillMode::eSolid)
					return vk::PolygonMode::eFill;
				else if (mode == FillMode::eWireframe)
					return vk::PolygonMode::eLine;

				assert(false && "NOT IMPLEMENTED STATE");
				return vk::PolygonMode::eFill;
			}

			inline vk::CullModeFlags CullModeToVk(CullModeFlags flags)
			{
				vk::CullModeFlags cull_mode{};

				if (flags & CullModeFlag::eFront)
					cull_mode |= vk::CullModeFlagBits::eFront;
				if (flags & CullModeFlag::eBack)
					cull_mode |= vk::CullModeFlagBits::eBack;

				return cull_mode;
			}

			inline vk::FrontFace FrontFaceToVk(FrontFace ff)
			{
				if (ff == FrontFace::eCCW)
					return vk::FrontFace::eCounterClockwise;
				else if (ff == FrontFace::eCW)
					return vk::FrontFace::eClockwise;

				assert(false && "NOT IMPLEMENTED STATE");
				return vk::FrontFace::eCounterClockwise;
			}

			inline vk::StencilOp StencilOpToVk(StencilOp op)
			{
				if (op == StencilOp::eKeep)
					return vk::StencilOp::eKeep;
				else if (op == StencilOp::eZero)
					return vk::StencilOp::eZero;
				else if (op == StencilOp::eReplace)
					return vk::StencilOp::eReplace;
				else if (op == StencilOp::eIncrementAndClamp)
					return vk::StencilOp::eIncrementAndClamp;
				else if (op == StencilOp::eDecrementAndClamp)
					return vk::StencilOp::eDecrementAndClamp;
				else if (op == StencilOp::eInvert)
					return vk::StencilOp::eInvert;
				else if (op == StencilOp::eIncrementAndWrap)
					return vk::StencilOp::eIncrementAndWrap;
				else if (op == StencilOp::eDecrementAndWrap)
					return vk::StencilOp::eDecrementAndWrap;

				assert(false && "NOT IMPLEMENTED STATE");
				return vk::StencilOp::eKeep;
			}

			inline vk::BlendFactor BlendFactorToVk(BlendFactor blend_factor)
			{
				if (blend_factor == BlendFactor::eZero)
					return vk::BlendFactor::eZero;
				else if (blend_factor == BlendFactor::eOne)
					return vk::BlendFactor::eOne;
				else if (blend_factor == BlendFactor::eSrcColor)
					return vk::BlendFactor::eSrcColor;
				else if (blend_factor == BlendFactor::eOneMinusSrcColor)
					return vk::BlendFactor::eOneMinusSrcColor;
				else if (blend_factor == BlendFactor::eDstColor)
					return vk::BlendFactor::eDstColor;
				else if (blend_factor == BlendFactor::eOneMinusDstColor)
					return vk::BlendFactor::eOneMinusDstColor;
				else if (blend_factor == BlendFactor::eSrcAlpha)
					return vk::BlendFactor::eSrcAlpha;
				else if (blend_factor == BlendFactor::eOneMinusSrcAlpha)
					return vk::BlendFactor::eOneMinusSrcAlpha;
				else if (blend_factor == BlendFactor::eDstAlpha)
					return vk::BlendFactor::eDstAlpha;
				else if (blend_factor == BlendFactor::eOneMinusDstAlpha)
					return vk::BlendFactor::eOneMinusDstAlpha;
				else if (blend_factor == BlendFactor::eConstantAlpha)
					return vk::BlendFactor::eConstantAlpha;
				else if (blend_factor == BlendFactor::eOneMinusConstantAlpha)
					return vk::BlendFactor::eOneMinusConstantAlpha;
				else if (blend_factor == BlendFactor::eSrcAlphaSaturate)
					return vk::BlendFactor::eSrcAlphaSaturate;
				
				return vk::BlendFactor::eZero;
			}

			inline vk::BlendOp BlendOpToVk(BlendOp blend_op)
			{
				if (blend_op == BlendOp::eAdd)
					return vk::BlendOp::eAdd;
				else if (blend_op == BlendOp::eSubtract)
					return vk::BlendOp::eSubtract;
				else if (blend_op == BlendOp::eReverseSubtract)
					return vk::BlendOp::eReverseSubtract;
				else if (blend_op == BlendOp::eMin)
					return vk::BlendOp::eMin;
				else if (blend_op == BlendOp::eMax)
					return vk::BlendOp::eMax;
				
				return vk::BlendOp::eAdd;
			}

			inline vk::ColorComponentFlags ColorMaskToVk(ColorMaskFlags flags)
			{
				vk::ColorComponentFlags vk_flags{};

				if (flags & ColorMaskFlag::eR)
					vk_flags |= vk::ColorComponentFlagBits::eR;
				if (flags & ColorMaskFlag::eG)
					vk_flags |= vk::ColorComponentFlagBits::eG;
				if (flags & ColorMaskFlag::eB)
					vk_flags |= vk::ColorComponentFlagBits::eB;
				if (flags & ColorMaskFlag::eA)
					vk_flags |= vk::ColorComponentFlagBits::eA;

				return vk_flags;
			}

			inline vk::AttachmentLoadOp LoadOpToVk(LoadOp op)
			{
				if (op == LoadOp::eLoad)
					return vk::AttachmentLoadOp::eLoad;
				else if (op == LoadOp::eClear)
					return vk::AttachmentLoadOp::eClear;
				else if (op == LoadOp::eDontCare)
					return vk::AttachmentLoadOp::eDontCare;

				return vk::AttachmentLoadOp::eDontCare;
			}

			inline vk::AttachmentStoreOp StoreOpToVk(StoreOp op)
			{
				if (op == StoreOp::eStore)
					return vk::AttachmentStoreOp::eStore;
				else if (op == StoreOp::eDontCare)
					return vk::AttachmentStoreOp::eDontCare;
				else if (op == StoreOp::eNone)
					return vk::AttachmentStoreOp::eNone;

				return vk::AttachmentStoreOp::eDontCare;
			}
		}
	}
}