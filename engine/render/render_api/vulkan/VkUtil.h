#pragma once

#include <GFXStructures.h>
#include <vulkan/vulkan.hpp>

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

		inline vk::DescriptorType DescriptorPoolToVk(DescriptorType type)
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

			return vk::DescriptorType::eSampler;
		}

		inline vk::Filter FilterToVk(Filter filter)
		{
			if (filter == Filter::eNearest)
				return vk::Filter::eNearest;
			else if (filter == Filter::eLinear)
				return vk::Filter::eLinear;

			return vk::Filter::eNearest;
		}

		inline vk::SamplerMipmapMode MipmapModeToVk(SamplerMipmapMode mode)
		{
			if (mode == SamplerMipmapMode::eNearest)
				return vk::SamplerMipmapMode::eNearest;
			else if (mode == SamplerMipmapMode::eLinear)
				return vk::SamplerMipmapMode::eLinear;

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

			return vk::QueryType::eOcclusion;
		}



		inline bool IsDepthOnlyFormat(vk::Format format)
		{
			return format == vk::Format::eD16Unorm ||
				format == vk::Format::eD32Sfloat;
		}

		inline bool IsDepthStencilFormat(vk::Format format)
		{
			return format == vk::Format::eD16UnormS8Uint ||
				format == vk::Format::eD24UnormS8Uint ||
				format == vk::Format::eD32SfloatS8Uint;
		}

		inline bool IsDepthFormat(vk::Format format)
		{
			return IsDepthOnlyFormat(format) || IsDepthStencilFormat(format);
		}

		inline bool IsStencilFormat(vk::Format format)
		{
			return !IsDepthOnlyFormat(format) || !IsDepthStencilFormat(format);
		}
	}
}