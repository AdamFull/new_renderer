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

	enum class QueueTypeFlag
	{
		eGraphics = 1 << 0,
		ePresent = 1 << 1,
		eCompute = 1 << 2,
		eTransfer = 1 << 3
	};
	UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(QueueTypeFlags, QueueTypeFlag);

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

	};
	UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(TextureUsageFlags, TextureUsageFlag);
}