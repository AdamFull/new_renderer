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
			eResourceDescriptor,
			eDescriptorAllocator,
			eSharedDescriptor,
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

		enum class ResourceStateFlag
		{
			eUndefined = 0,
			eRead = 1 << 0,
			eWrite = 1 << 1,
			eCompute = 1 << 2,
			eGraphics = 1 << 3,
			eAttachment = 1 << 4,
			eDepth = 1 << 5,
			eStencil = 1 << 6,
			eSample = 1 << 7,
			eStorage = 1 << 8,
			eCopy = 1 << 9,
			ePresent = 1 << 10,

			// Rendering states
			eAttachmentRead = eRead | eAttachment,
			eAttachmentWrite = eWrite | eAttachment,

			// Depth targets
			eDepthAttachmentRead = eAttachmentRead | eDepth,
			eDepthAttachmentWrite = eAttachmentWrite | eDepth,

			eStencilAttachmentRead = eAttachmentRead | eStencil,
			eStencilAttachmentWrite = eAttachmentWrite | eStencil,

			eDepthStencilAttachmentRead = eAttachmentRead | eDepth | eStencil,
			eDepthStencilAttachmentWrite = eAttachmentWrite | eDepth | eStencil,

			eShaderReadOnlyResource = eRead | eCompute | eGraphics | eSample,

			// Copy states
			eCopyDst = eWrite | eCopy,
			eCopySrc = eRead | eCopy,

			// Present states
			ePresentSrc = eRead | ePresent
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(ResourceStateFlags, ResourceStateFlag);

		enum class AccessTypeFlag
		{
			eRead = 1 << 0,
			eWrite = 1 << 1,
			eRW = eRead | eWrite
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(AccessTypeFlags, AccessTypeFlag);

		// For sampler
		enum class Filter
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

		enum class PrimitiveTopology
		{
			ePointList,
			eLineList,
			eLineStrip,
			eTriangleList,
			eTriangleStrip,
			ePatchList
		};

		enum class SampleCount
		{
			e1 = 1,
			e2 = 2,
			e4 = 4,
			e8 = 8,
			e16 = 16
		};

		enum class FillMode
		{
			eSolid,
			eWireframe
		};

		enum class CullModeFlag
		{
			eNone = 0,
			eFront = 1 << 0,
			eBack = 1 << 1
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(CullModeFlags, CullModeFlag);

		enum class FrontFace
		{
			eCCW,
			eCW
		};

		enum class StencilOp
		{
			eKeep,
			eZero,
			eReplace,
			eIncrementAndClamp,
			eDecrementAndClamp,
			eInvert,
			eIncrementAndWrap,
			eDecrementAndWrap
		};

		enum class BlendFactor
		{
			eZero,
			eOne,
			eSrcColor,
			eOneMinusSrcColor,
			eDstColor ,
			eOneMinusDstColor,
			eSrcAlpha,
			eOneMinusSrcAlpha,
			eDstAlpha,
			eOneMinusDstAlpha,
			eConstantAlpha,
			eOneMinusConstantAlpha,
			eSrcAlphaSaturate
		};

		enum class BlendOp
		{
			eAdd,
			eSubtract,
			eReverseSubtract,
			eMin,
			eMax
		};

		enum class ColorMaskFlag
		{
			eR = 1 << 0,
			eG = 1 << 1,
			eB = 1 << 2,
			eA = 1 << 3,
			eAll = eR | eG | eB | eA
		};
		UTL_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(ColorMaskFlags, ColorMaskFlag);

		enum class LoadOp
		{
			eLoad,
			eClear,
			eDontCare
		};

		enum class StoreOp
		{
			eStore,
			eDontCare,
			eNone
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