#pragma once

#include <enum.hpp>

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

	enum class ShaderStage
	{
		eVertex,
		eTesselationControl,
		eTesselationEvaluation,
		eGeometry,
		eFragment,
		eCompute
	};

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
}