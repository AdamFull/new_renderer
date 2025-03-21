#pragma once

#include "../util/utl_enum.hpp"

namespace edge::gfx
{
	enum class RenderApi {
		eAuto,
		eVulkan,
		eDirectx12
	};

	enum class SemaphoreType {
		eBinary,
		eTimeline
	};

	enum class ResourceType {
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

	enum class ShaderStageFlag {
		eVertex = 1 << 0,
		eTessellationControl = 1 << 1,
		eTessellationEvaluation = 1 << 2,
		eGeometry = 1 << 3,
		eFragment = 1 << 4,
		eCompute = 1 << 5,
		eAllGraphics = eVertex | eTessellationControl | eTessellationEvaluation | eGeometry | eFragment
	};

	enum class IndexType {
		eUint16,
		eUint32
	};

	enum class QueueTypeFlag {
		eGraphics = 1 << 0,
		ePresent = 1 << 1,
		eCompute = 1 << 2,
		eTransfer = 1 << 3
	};

	enum class CommandPoolResetMode {
		eResetPool,
		eResetIndividually,
		eAlwaysAllocate,
	};

	enum class StageFlag {
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

	enum class ResourceStateFlag {
		eUndefined = 0,
		eRead = 1 << 0,
		eWrite = 1 << 1,
		eCompute = 1 << 2,
		eGraphics = 1 << 3,
		eColorAttachment = 1 << 4,
		eDepthAttachment = 1 << 5,
		eStencilAttachment = 1 << 6,
		eIndex = 1 << 7,
		eVertex = 1 << 8,
		eSample = 1 << 9,
		eStorage = 1 << 10,
		eIndirect = 1 << 11,
		eGeneral = 1 << 12,
		eCopy = 1 << 13,
		ePresent = 1 << 14,

		eIndexRead = eRead | eGraphics | eIndex,
		eVertexRead = eRead | eGraphics | eVertex,

		// Rendering states
		eColorAttachmentRead = eRead | eColorAttachment,
		eColorAttachmentWrite = eWrite | eColorAttachment,

		// Depth targets
		eDepthAttachmentRead = eRead | eDepthAttachment,
		eDepthAttachmentWrite = eWrite | eDepthAttachment,

		eStencilAttachmentRead = eRead | eStencilAttachment,
		eStencilAttachmentWrite = eWrite | eStencilAttachment,

		eDepthStencilAttachmentRead = eRead | eDepthAttachment | eStencilAttachment,
		eDepthStencilAttachmentWrite = eWrite | eDepthAttachment | eStencilAttachment,

		eShaderReadOnlyResource = eRead | eCompute | eGraphics | eSample,

		// Copy states
		eCopyDst = eWrite | eCopy,
		eCopySrc = eRead | eCopy,

		// Present states
		ePresentSrc = eRead | ePresent
	};

	enum class AccessTypeFlag {
		eRead = 1 << 0,
		eWrite = 1 << 1,
		eRW = eRead | eWrite
	};

	// For sampler
	enum class Filter {
		eNearest,
		eLinear
	};

	enum class SamplerAddressMode {
		eMirror,
		eRepeat,
		eClampToEdge,
		eClampToBorder
	};


	enum class CompareOp {
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
	enum class QueryType {
		eTimestamp,
		eOcclusion,
		ePipelineStatistics
	};

	enum class BufferUsageFlag {
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
		eVertexBufferCPU = eCopy | eVertex | eCpuToGpu,
		eVertexBuffer = eCopy | eVertex | eGpuOnly,
		eIndexBufferCPU = eCopy | eIndex | eCpuToGpu,
		eIndexBuffer = eCopy | eIndex | eGpuOnly,
		eUniformBuffer = eCopy | eUniform | eCpuToGpu,
		eIndirectBuffer = eCopy | eStorage | eIndirect | eGpuOnly
	};

	enum class TextureUsageFlag {
		eCubemap = 1 << 0,
		eTransferable = 1 << 1,
		eSamplable = 1 << 2,
		eStorage = 1 << 3,
		eRenderTarget = 1 << 4
	};

	enum class PipelineType {
		eCompute,
		eGraphics
	};

	enum class PrimitiveTopology {
		ePointList,
		eLineList,
		eLineStrip,
		eTriangleList,
		eTriangleStrip,
		ePatchList
	};

	enum class SampleCount {
		e1 = 1,
		e2 = 2,
		e4 = 4,
		e8 = 8,
		e16 = 16
	};

	enum class FillMode {
		eSolid,
		eWireframe
	};

	enum class CullModeFlag {
		eNone = 0,
		eFront = 1 << 0,
		eBack = 1 << 1
	};

	enum class FrontFace {
		eCCW,
		eCW
	};

	enum class StencilOp {
		eKeep,
		eZero,
		eReplace,
		eIncrementAndClamp,
		eDecrementAndClamp,
		eInvert,
		eIncrementAndWrap,
		eDecrementAndWrap
	};

	enum class BlendFactor {
		eZero,
		eOne,
		eSrcColor,
		eOneMinusSrcColor,
		eDstColor,
		eOneMinusDstColor,
		eSrcAlpha,
		eOneMinusSrcAlpha,
		eDstAlpha,
		eOneMinusDstAlpha,
		eConstantAlpha,
		eOneMinusConstantAlpha,
		eSrcAlphaSaturate
	};

	enum class BlendOp {
		eAdd,
		eSubtract,
		eReverseSubtract,
		eMin,
		eMax
	};

	enum class ColorMaskFlag {
		eR = 1 << 0,
		eG = 1 << 1,
		eB = 1 << 2,
		eA = 1 << 3,
		eAll = eR | eG | eB | eA
	};

	enum class LoadOp {
		eLoad,
		eClear,
		eDontCare
	};

	enum class StoreOp {
		eStore,
		eDontCare,
		eNone
	};

	enum class DescriptorType {
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

// Meta data registration
EDGE_REGISTER_ENUM_CLASS(edge::gfx, RenderApi,
	EDGE_ENUM_META(edge::gfx::RenderApi::eAuto, "Auto"),
	EDGE_ENUM_META(edge::gfx::RenderApi::eVulkan, "Vulkan"),
	EDGE_ENUM_META(edge::gfx::RenderApi::eDirectx12, "Directx12")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, ResourceType,
	EDGE_ENUM_META(edge::gfx::ResourceType::eUnknown, "Auto"),
	EDGE_ENUM_META(edge::gfx::ResourceType::eFence, "IFence"),
	EDGE_ENUM_META(edge::gfx::ResourceType::eSemaphore, "ISemaphore"),
	EDGE_ENUM_META(edge::gfx::ResourceType::eQueue, "Queue"),
	EDGE_ENUM_META(edge::gfx::ResourceType::eSwapchain, "ISwapchain"),
	EDGE_ENUM_META(edge::gfx::ResourceType::eBuffer, "Buffer"),
	EDGE_ENUM_META(edge::gfx::ResourceType::eCommandList, "ICommandList"),
	EDGE_ENUM_META(edge::gfx::ResourceType::eCommandPool, "ICommandPool"),
	EDGE_ENUM_META(edge::gfx::ResourceType::eTexture, "Texture"),
	EDGE_ENUM_META(edge::gfx::ResourceType::eSampler, "ISampler"),
	EDGE_ENUM_META(edge::gfx::ResourceType::eShader, "Shader"),
	EDGE_ENUM_META(edge::gfx::ResourceType::eRootSignature, "IRootSignature"),
	EDGE_ENUM_META(edge::gfx::ResourceType::ePipeline, "IPipeline"),
	EDGE_ENUM_META(edge::gfx::ResourceType::ePipelineCache, "IPipelineCache"),
	EDGE_ENUM_META(edge::gfx::ResourceType::eResourceDescriptor, "IResourceDescriptor"),
	EDGE_ENUM_META(edge::gfx::ResourceType::eDescriptorAllocator, "IDescriptorAllocator"),
	EDGE_ENUM_META(edge::gfx::ResourceType::eSharedDescriptor, "ISharedDescriptor"),
	EDGE_ENUM_META(edge::gfx::ResourceType::eQueryPool, "IQueryPool")
)

EDGE_REGISTER_ENUM_FLAGS(edge::gfx, ShaderStageFlags, ShaderStageFlag,
	EDGE_ENUM_META(edge::gfx::ShaderStageFlag::eVertex, "Vertex"),
	EDGE_ENUM_META(edge::gfx::ShaderStageFlag::eTessellationControl, "TessellationControl"),
	EDGE_ENUM_META(edge::gfx::ShaderStageFlag::eTessellationEvaluation, "TessellationEvaluation"),
	EDGE_ENUM_META(edge::gfx::ShaderStageFlag::eGeometry, "Geometry"),
	EDGE_ENUM_META(edge::gfx::ShaderStageFlag::eFragment, "Fragment"),
	EDGE_ENUM_META(edge::gfx::ShaderStageFlag::eCompute, "Compute"),
	EDGE_ENUM_META(edge::gfx::ShaderStageFlag::eAllGraphics, "AllGraphics")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, IndexType,
	EDGE_ENUM_META(edge::gfx::IndexType::eUint16, "Uint16"),
	EDGE_ENUM_META(edge::gfx::IndexType::eUint32, "Uint32")
)

EDGE_REGISTER_ENUM_FLAGS(edge::gfx, QueueTypeFlags, QueueTypeFlag,
	EDGE_ENUM_META(edge::gfx::QueueTypeFlag::eGraphics, "Graphics"),
	EDGE_ENUM_META(edge::gfx::QueueTypeFlag::ePresent, "Present"),
	EDGE_ENUM_META(edge::gfx::QueueTypeFlag::eCompute, "Compute"),
	EDGE_ENUM_META(edge::gfx::QueueTypeFlag::eTransfer, "Transfer")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, CommandPoolResetMode,
	EDGE_ENUM_META(edge::gfx::CommandPoolResetMode::eResetPool, "ResetPool"),
	EDGE_ENUM_META(edge::gfx::CommandPoolResetMode::eResetIndividually, "ResetIndividually"),
	EDGE_ENUM_META(edge::gfx::CommandPoolResetMode::eAlwaysAllocate, "AlwaysAllocate")
)

EDGE_REGISTER_ENUM_FLAGS(edge::gfx, StageFlags, StageFlag,
	EDGE_ENUM_META(edge::gfx::StageFlag::eNone, "None"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eTopOfPipe, "TopOfPipe"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eDrawIndirect, "draw_indirect"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eVertexInput, "VertexInput"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eVertexShader, "VertexShader"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eTessellationControlShader, "TessellationControlShader"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eTessellationEvaluationShader, "TessellationEvaluationShader"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eGeometryShader, "GeometryShader"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eFragmentShader, "FragmentShader"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eEarlyFragmentTests, "EarlyFragmentTests"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eLateFragmentTests, "LateFragmentTests"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eColorAttachmentOutput, "ColorAttachmentOutput"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eComputeShader, "ComputeShader"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eAllTransfer, "AllTransfer"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eTransfer, "Transfer"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eBottomOfPipe, "BottomOfPipe"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eHost, "Host"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eAllGraphics, "AllGraphics"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eAllCommands, "AllCommands"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eCopy, "Copy"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eResolve, "Resolve"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eBlit, "Blit"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eClear, "Clear"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eIndexInput, "IndexInput"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eVertexAttributeInput, "VertexAttributeInput"),
	EDGE_ENUM_META(edge::gfx::StageFlag::ePreRasterizationShaders, "PreRasterizationShaders")
)

EDGE_REGISTER_ENUM_FLAGS(edge::gfx, ResourceStateFlags, ResourceStateFlag,
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eUndefined, "Undefined"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eRead, "Read"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eWrite, "Write"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eCompute, "Compute"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eGraphics, "Graphics"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eColorAttachment, "ColorAttachment"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eDepthAttachment, "DepthAttachment"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eStencilAttachment, "StencilAttachment"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eIndex, "Index"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eVertex, "Vertex"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eSample, "Sample"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eStorage, "Storage"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eIndirect, "Indirect"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eGeneral, "General"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eCopy, "Copy"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::ePresent, "Present"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eColorAttachmentRead, "ColorAttachmentRead"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eColorAttachmentWrite, "ColorAttachmentWrite"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eDepthAttachmentRead, "DepthAttachmentRead"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eDepthAttachmentWrite, "DepthAttachmentWrite"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eStencilAttachmentRead, "StencilAttachmentRead"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eStencilAttachmentWrite, "StencilAttachmentWrite"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eDepthStencilAttachmentRead, "DepthStencilAttachmentRead"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eDepthStencilAttachmentWrite, "DepthStencilAttachmentWrite"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eShaderReadOnlyResource, "ShaderReadOnlyResource"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eCopyDst, "CopyDst"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eCopySrc, "CopySrc"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::ePresentSrc, "PresentSrc")
)

EDGE_REGISTER_ENUM_FLAGS(edge::gfx, AccessTypeFlags, AccessTypeFlag,
	EDGE_ENUM_META(edge::gfx::AccessTypeFlag::eRead, "Read"),
	EDGE_ENUM_META(edge::gfx::AccessTypeFlag::eWrite, "Write"),
	EDGE_ENUM_META(edge::gfx::AccessTypeFlag::eRW, "RW")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, Filter,
	EDGE_ENUM_META(edge::gfx::Filter::eNearest, "Nearest"),
	EDGE_ENUM_META(edge::gfx::Filter::eLinear, "Linear")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, SamplerAddressMode,
	EDGE_ENUM_META(edge::gfx::SamplerAddressMode::eMirror, "Mirror"),
	EDGE_ENUM_META(edge::gfx::SamplerAddressMode::eRepeat, "Repeat"),
	EDGE_ENUM_META(edge::gfx::SamplerAddressMode::eClampToEdge, "ClampToEdge"),
	EDGE_ENUM_META(edge::gfx::SamplerAddressMode::eClampToBorder, "ClampToBorder")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, CompareOp,
	EDGE_ENUM_META(edge::gfx::CompareOp::eNone, "None"),
	EDGE_ENUM_META(edge::gfx::CompareOp::eNever, "Never"),
	EDGE_ENUM_META(edge::gfx::CompareOp::eLess, "Less"),
	EDGE_ENUM_META(edge::gfx::CompareOp::eEqual, "Equal"),
	EDGE_ENUM_META(edge::gfx::CompareOp::eLessOrEqual, "LessOrEqual"),
	EDGE_ENUM_META(edge::gfx::CompareOp::eGreater, "Greater"),
	EDGE_ENUM_META(edge::gfx::CompareOp::eNotEqual, "NotEqual"),
	EDGE_ENUM_META(edge::gfx::CompareOp::eGreaterOrEqual, "GreaterOrEqual"),
	EDGE_ENUM_META(edge::gfx::CompareOp::eAlways, "Always")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, QueryType,
	EDGE_ENUM_META(edge::gfx::QueryType::eTimestamp, "Timestamp"),
	EDGE_ENUM_META(edge::gfx::QueryType::eOcclusion, "Occlusion"),
	EDGE_ENUM_META(edge::gfx::QueryType::ePipelineStatistics, "PipelineStatistics")
)

EDGE_REGISTER_ENUM_FLAGS(edge::gfx, BufferUsageFlags, BufferUsageFlag,
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eCopy, "Copy"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eStorage, "Storage"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eVertex, "Vertex"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eIndex, "Index"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eUniform, "Uniform"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eIndirect, "Indirect"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eGpuOnly, "GpuOnly"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eCpuToGpu, "CpuToGpu"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eHasDeviceAddress, "HasDeviceAddress"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eStagingBuffer, "StagingBuffer"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eStorageBufferCPU, "StorageBufferCPU"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eStorageBuffer, "StorageBuffer"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eVertexBuffer, "VertexBuffer"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eIndexBuffer, "IndexBuffer"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eUniformBuffer, "UniformBuffer"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eIndirectBuffer, "IndirectBuffer")
)

EDGE_REGISTER_ENUM_FLAGS(edge::gfx, TextureUsageFlags, TextureUsageFlag,
	EDGE_ENUM_META(edge::gfx::TextureUsageFlag::eCubemap, "Cubemap"),
	EDGE_ENUM_META(edge::gfx::TextureUsageFlag::eTransferable, "Transferable"),
	EDGE_ENUM_META(edge::gfx::TextureUsageFlag::eSamplable, "Samplable"),
	EDGE_ENUM_META(edge::gfx::TextureUsageFlag::eStorage, "Storage"),
	EDGE_ENUM_META(edge::gfx::TextureUsageFlag::eRenderTarget, "RenderTarget")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, PipelineType,
	EDGE_ENUM_META(edge::gfx::PipelineType::eCompute, "Compute"),
	EDGE_ENUM_META(edge::gfx::PipelineType::eGraphics, "Graphics")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, PrimitiveTopology,
	EDGE_ENUM_META(edge::gfx::PrimitiveTopology::ePointList, "PointList"),
	EDGE_ENUM_META(edge::gfx::PrimitiveTopology::eLineList, "LineList"),
	EDGE_ENUM_META(edge::gfx::PrimitiveTopology::eLineStrip, "LineStrip"),
	EDGE_ENUM_META(edge::gfx::PrimitiveTopology::eTriangleList, "TriangleList"),
	EDGE_ENUM_META(edge::gfx::PrimitiveTopology::eTriangleStrip, "TriangleStrip"),
	EDGE_ENUM_META(edge::gfx::PrimitiveTopology::ePatchList, "PatchList")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, SampleCount,
	EDGE_ENUM_META(edge::gfx::SampleCount::e1, "1"),
	EDGE_ENUM_META(edge::gfx::SampleCount::e2, "2"),
	EDGE_ENUM_META(edge::gfx::SampleCount::e4, "4"),
	EDGE_ENUM_META(edge::gfx::SampleCount::e8, "8"),
	EDGE_ENUM_META(edge::gfx::SampleCount::e16, "16")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, FillMode,
	EDGE_ENUM_META(edge::gfx::FillMode::eSolid, "Solid"),
	EDGE_ENUM_META(edge::gfx::FillMode::eWireframe, "Wireframe")
)

EDGE_REGISTER_ENUM_FLAGS(edge::gfx, CullModeFlags, CullModeFlag,
	EDGE_ENUM_META(edge::gfx::CullModeFlag::eNone, "None"),
	EDGE_ENUM_META(edge::gfx::CullModeFlag::eFront, "Front"),
	EDGE_ENUM_META(edge::gfx::CullModeFlag::eBack, "Back")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, FrontFace,
	EDGE_ENUM_META(edge::gfx::FrontFace::eCCW, "CCW"),
	EDGE_ENUM_META(edge::gfx::FrontFace::eCW, "CW")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, StencilOp,
	EDGE_ENUM_META(edge::gfx::StencilOp::eKeep, "Keep"),
	EDGE_ENUM_META(edge::gfx::StencilOp::eZero, "Zero"),
	EDGE_ENUM_META(edge::gfx::StencilOp::eReplace, "Replace"),
	EDGE_ENUM_META(edge::gfx::StencilOp::eIncrementAndClamp, "IncrementAndClamp"),
	EDGE_ENUM_META(edge::gfx::StencilOp::eDecrementAndClamp, "DecrementAndClamp"),
	EDGE_ENUM_META(edge::gfx::StencilOp::eInvert, "Invert"),
	EDGE_ENUM_META(edge::gfx::StencilOp::eIncrementAndWrap, "IncrementAndWrap"),
	EDGE_ENUM_META(edge::gfx::StencilOp::eDecrementAndWrap, "DecrementAndWrap")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, BlendFactor,
	EDGE_ENUM_META(edge::gfx::BlendFactor::eZero, "Zero"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eOne, "One"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eSrcColor, "SrcColor"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eOneMinusSrcColor, "OneMinusSrcColor"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eDstColor, "DstColor"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eOneMinusDstColor, "OneMinusDstColor"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eSrcAlpha, "SrcAlpha"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eOneMinusSrcAlpha, "OneMinusSrcAlpha"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eOneMinusSrcAlpha, "OneMinusSrcAlpha"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eDstAlpha, "DstAlpha"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eOneMinusDstAlpha, "OneMinusDstAlpha"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eConstantAlpha, "ConstantAlpha"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eOneMinusConstantAlpha, "OneMinusConstantAlpha"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eSrcAlphaSaturate, "SrcAlphaSaturate")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, BlendOp,
	EDGE_ENUM_META(edge::gfx::BlendOp::eAdd, "Add"),
	EDGE_ENUM_META(edge::gfx::BlendOp::eSubtract, "Subtract"),
	EDGE_ENUM_META(edge::gfx::BlendOp::eReverseSubtract, "ReverseSubtract"),
	EDGE_ENUM_META(edge::gfx::BlendOp::eMin, "Min"),
	EDGE_ENUM_META(edge::gfx::BlendOp::eMax, "Max")
)

EDGE_REGISTER_ENUM_FLAGS(edge::gfx, ColorMaskFlags, ColorMaskFlag,
	EDGE_ENUM_META(edge::gfx::ColorMaskFlag::eR, "R"),
	EDGE_ENUM_META(edge::gfx::ColorMaskFlag::eG, "G"),
	EDGE_ENUM_META(edge::gfx::ColorMaskFlag::eB, "B"),
	EDGE_ENUM_META(edge::gfx::ColorMaskFlag::eA, "A"),
	EDGE_ENUM_META(edge::gfx::ColorMaskFlag::eAll, "All")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, LoadOp,
	EDGE_ENUM_META(edge::gfx::LoadOp::eLoad, "Load"),
	EDGE_ENUM_META(edge::gfx::LoadOp::eClear, "Clear"),
	EDGE_ENUM_META(edge::gfx::LoadOp::eDontCare, "DontCare")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, StoreOp,
	EDGE_ENUM_META(edge::gfx::StoreOp::eStore, "Store"),
	EDGE_ENUM_META(edge::gfx::StoreOp::eDontCare, "DontCare"),
	EDGE_ENUM_META(edge::gfx::StoreOp::eNone, "None")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, DescriptorType,
	EDGE_ENUM_META(edge::gfx::DescriptorType::eSampler, "ISampler"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eCombinedImageSampler, "CombinedImageSampler"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eSampledImage, "SampledImage"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eStorageImage, "StorageImage"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eUniformTexelBuffer, "UniformTexelBuffer"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eStorageTexelBuffer, "StorageTexelBuffer"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eUniformBuffer, "UniformBuffer"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eStorageBuffer, "StorageBuffer"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eUniformBufferDynamic, "UniformBufferDynamic"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eStorageBufferDynamic, "StorageBufferDynamic"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eInputAttachment, "InputAttachment")
)

namespace edge::gfx
{
	void to_json(nlohmann::json& json, const ShaderStageFlags& type);
	void from_json(const nlohmann::json& json, ShaderStageFlags& type);

	void to_json(nlohmann::json& json, const QueueTypeFlags& type);
	void from_json(const nlohmann::json& json, QueueTypeFlags& type);

	void to_json(nlohmann::json& json, const AccessTypeFlags& type);
	void from_json(const nlohmann::json& json, AccessTypeFlags& type);

	void to_json(nlohmann::json& json, const BufferUsageFlags& type);
	void from_json(const nlohmann::json& json, BufferUsageFlags& type);

	void to_json(nlohmann::json& json, const TextureUsageFlags& type);
	void from_json(const nlohmann::json& json, TextureUsageFlags& type);

	void to_json(nlohmann::json& json, const CullModeFlags& type);
	void from_json(const nlohmann::json& json, CullModeFlags& type);

	void to_json(nlohmann::json& json, const ColorMaskFlags& type);
	void from_json(const nlohmann::json& json, ColorMaskFlags& type);
}