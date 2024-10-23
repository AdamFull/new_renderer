#pragma once

#include <enum.hpp>

namespace engine
{
	namespace gfx
	{
		enum class RenderApi
		{
			eAuto,
			eVulkan,
			eDirectx12
		};

		enum class ResourceType
		{
			eUnknown,
			eFence,
			eSemaphore,
			eQueue,
			eSwapchain,
			eBuffer,
			eCommandList,
			eCommandPool,
			eTexture,
			eSampler,
			eShader,
			eRootSignature,
			ePipeline,
			ePipelineCache,
			eDescriptorPool,
			eDescriptorSet,
			eQueryPool
		};

		enum class ShaderStageFlag
		{
			eVertex = 1 << 0,
			eTessellationControl = 1 << 1,
			eTessellationEvaluation = 1 << 2,
			eGeometry = 1 << 3,
			eFragment = 1 << 4,
			eCompute = 1 << 5,
			eAllGraphics = eVertex | eTessellationControl | eTessellationEvaluation | eGeometry | eFragment
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(ShaderStageFlags, ShaderStageFlag);

		enum class IndexType
		{
			eUint16,
			eUint32
		};

		enum class QueueTypeFlag
		{
			eGraphics = 1 << 0,
			ePresent = 1 << 1,
			eCompute = 1 << 2,
			eTransfer = 1 << 3
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(QueueTypeFlags, QueueTypeFlag);

		enum class CommandPoolResetMode
		{
			eResetPool,
			eResetIndividually,
			eAlwaysAllocate,
		};

		enum class StageFlag
		{
			eNone = 1 << 0,
			eTopOfPipe = 1 << 1,
			eDrawIndirect = 1 << 2,
			eVertexInput = 1 << 3,
			eVertexShader = 1 << 4,
			eTessellationControlShader = 1 << 5,
			eTessellationEvaluationShader = 1 << 6,
			eGeometryShader = 1 << 7,
			eFragmentShader = 1 << 8,
			eEarlyFragmentTests = 1 << 9,
			eLateFragmentTests = 1 << 10,
			eColorAttachmentOutput = 1 << 11,
			eComputeShader = 1 << 12,
			eAllTransfer = 1 << 13,
			eTransfer = 1 << 14,
			eBottomOfPipe = 1 << 15,
			eHost = 1 << 16,
			eAllGraphics = 1 << 17,
			eAllCommands = 1 << 18,
			eCopy = 1 << 19,
			eResolve = 1 << 20,
			eBlit = 1 << 21,
			eClear = 1 << 22,
			eIndexInput = 1 << 23,
			eVertexAttributeInput = 1 << 24,
			ePreRasterizationShaders = 1 << 25
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(StageFlags, StageFlag);

		enum class AccessFlag
		{
			eNone = 1 << 0,
			eIndirectCommandRead = 1 << 1,
			eIndexRead = 1 << 2,
			eVertexAttributeRead = 1 << 3,
			eUniformRead = 1 << 4,
			eInputAttachmentRead = 1 << 5,
			eShaderRead = 1 << 6,
			eShaderWrite = 1 << 7,
			eColorAttachmentRead = 1 << 8,
			eColorAttachmentWrite = 1 << 9,
			eDepthStencilAttachmentRead = 1 << 10,
			eDepthStencilAttachmentWrite = 1 << 11,
			eTransferRead = 1 << 12,
			eTransferWrite = 1 << 13,
			eHostRead = 1 << 14,
			eHostWrite = 1 << 15,
			eMemoryRead = 1 << 16,
			eMemoryWrite = 1 << 17,
			eShaderSampledRead = 1 << 18,
			eShaderStorageRead = 1 << 19,
			eShaderStorageWrite = 1 << 20
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(AccessFlags, AccessFlag);

		// For sampler
		enum class Filter
		{
			eNearest,
			eLinear
		};

		enum class SamplerMipmapMode
		{
			eNearest,
			eLinear
		};

		enum class SamplerAddressMode
		{
			eMirror,
			eRepeat,
			eClampToEdge,
			eClampToBorder
		};


		enum class CompareOp
		{
			eNone,
			eNever,
			eLess,
			eEqual,
			eLessOrEqual,
			eGreater,
			eNotEqual,
			eGreaterOrEqual,
			eAlways
		};

		// For query pool
		enum class QueryType
		{
			eTimestamp,
			eOcclusion,
			ePipelineStatistics
		};

		enum class BufferUsageFlag
		{
			eCopy = 1 << 0,
			eStorage = 1 << 1,
			eVertex = 1 << 2,
			eIndex = 1 << 3,
			eUniform = 1 << 4,
			eIndirect = 1 << 5,
			eGpuOnly = 1 << 6,
			eCpuToGpu = 1 << 7,

			eHasDeviceAddress = eVertex | eIndex | eStorage | eIndirect,

			eStagingBuffer = eCopy,
			eStorageBufferCPU = eCopy | eStorage | eCpuToGpu,
			eStorageBuffer = eCopy | eStorage | eGpuOnly,
			eVertexBuffer = eCopy | eVertex | eGpuOnly,
			eIndexBuffer = eCopy | eIndex | eGpuOnly,
			eUniformBuffer = eCopy | eUniform | eCpuToGpu,
			eIndirectBuffer = eCopy | eStorage | eIndirect | eGpuOnly
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(BufferUsageFlags, BufferUsageFlag);

		enum class TextureUsageFlag
		{
			eCubemap = 1 << 0,
			eTransferable = 1 << 1,
			eSamplable = 1 << 2,
			eStorage = 1 << 3,
			eRenderTarget = 1 << 4
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(TextureUsageFlags, TextureUsageFlag);

		enum class PipelineType
		{
			eCompute,
			eGraphics
		};

		enum class DescriptorType
		{
			eSampler,
			eCombinedImageSampler,
			eSampledImage,
			eStorageImage,
			eUniformTexelBuffer,
			eStorageTexelBuffer,
			eUniformBuffer,
			eStorageBuffer,
			eUniformBufferDynamic,
			eStorageBufferDynamic,
			eInputAttachment,
		};
	}
}