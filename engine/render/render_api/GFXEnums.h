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
		eRenderTarget,
		eSampler,
		eShader,
		eRootSignature,
		ePipeline,
		ePipelineCache,
		eDescriptorSet,
		eQueryPool
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
}