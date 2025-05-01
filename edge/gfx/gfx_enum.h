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
		eDescriptorLayout,
		eBindingLayout,
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
		eAllGraphics = eVertex | eTessellationControl | eTessellationEvaluation | eGeometry | eFragment,
		eAll = eAllGraphics | eCompute
	};

	enum class IndexType {
		eUint16,
		eUint32
	};

	enum class QueueType {
		eDirect,
		eCompute,
		eCopy
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
		eUnknown,
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
	EDGE_ENUM_META(edge::gfx::RenderApi::eAuto, "auto"),
	EDGE_ENUM_META(edge::gfx::RenderApi::eVulkan, "vulkan"),
	EDGE_ENUM_META(edge::gfx::RenderApi::eDirectx12, "directx12")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, ResourceType,
	EDGE_ENUM_META(edge::gfx::ResourceType::eUnknown, "auto"),
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
	EDGE_ENUM_META(edge::gfx::ResourceType::eDescriptorAllocator, "IDescriptorAllocator"),
	EDGE_ENUM_META(edge::gfx::ResourceType::eSharedDescriptor, "ISharedDescriptor"),
	EDGE_ENUM_META(edge::gfx::ResourceType::eQueryPool, "IQueryPool")
)

EDGE_REGISTER_ENUM_FLAGS(edge::gfx, ShaderStageFlags, ShaderStageFlag,
	EDGE_ENUM_META(edge::gfx::ShaderStageFlag::eVertex, "vertex"),
	EDGE_ENUM_META(edge::gfx::ShaderStageFlag::eTessellationControl, "tessellation_control"),
	EDGE_ENUM_META(edge::gfx::ShaderStageFlag::eTessellationEvaluation, "tessellation_evaluation"),
	EDGE_ENUM_META(edge::gfx::ShaderStageFlag::eGeometry, "geometry"),
	EDGE_ENUM_META(edge::gfx::ShaderStageFlag::eFragment, "fragment"),
	EDGE_ENUM_META(edge::gfx::ShaderStageFlag::eCompute, "compute"),
	EDGE_ENUM_META(edge::gfx::ShaderStageFlag::eAllGraphics, "all_graphics")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, IndexType,
	EDGE_ENUM_META(edge::gfx::IndexType::eUint16, "uint16"),
	EDGE_ENUM_META(edge::gfx::IndexType::eUint32, "uint32")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, QueueType,
	EDGE_ENUM_META(edge::gfx::QueueType::eDirect, "direct"),
	EDGE_ENUM_META(edge::gfx::QueueType::eCompute, "compute"),
	EDGE_ENUM_META(edge::gfx::QueueType::eCopy, "copy")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, CommandPoolResetMode,
	EDGE_ENUM_META(edge::gfx::CommandPoolResetMode::eResetPool, "reset_pool"),
	EDGE_ENUM_META(edge::gfx::CommandPoolResetMode::eResetIndividually, "reset_individually"),
	EDGE_ENUM_META(edge::gfx::CommandPoolResetMode::eAlwaysAllocate, "always_allocate")
)

EDGE_REGISTER_ENUM_FLAGS(edge::gfx, StageFlags, StageFlag,
	EDGE_ENUM_META(edge::gfx::StageFlag::eNone, "none"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eTopOfPipe, "top_of_pipe"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eDrawIndirect, "draw_indirect"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eVertexInput, "vertex_input"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eVertexShader, "vertex_shader"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eTessellationControlShader, "tessellation_control_shader"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eTessellationEvaluationShader, "tessellation_evaluation_shader"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eGeometryShader, "geometry_shader"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eFragmentShader, "fragment_shader"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eEarlyFragmentTests, "early_fragment_tests"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eLateFragmentTests, "late_fragment_tests"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eColorAttachmentOutput, "color_attachment_output"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eComputeShader, "compute_shader"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eAllTransfer, "all_transfer"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eTransfer, "transfer"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eBottomOfPipe, "bottom_of_pipe"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eHost, "host"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eAllGraphics, "all_graphics"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eAllCommands, "all_commands"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eCopy, "copy"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eResolve, "resolve"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eBlit, "blit"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eClear, "clear"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eIndexInput, "index_input"),
	EDGE_ENUM_META(edge::gfx::StageFlag::eVertexAttributeInput, "vertex_attribute_input"),
	EDGE_ENUM_META(edge::gfx::StageFlag::ePreRasterizationShaders, "pre_rasterization_shaders")
)

EDGE_REGISTER_ENUM_FLAGS(edge::gfx, ResourceStateFlags, ResourceStateFlag,
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eUndefined, "undefined"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eRead, "read"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eWrite, "write"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eCompute, "compute"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eGraphics, "graphics"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eColorAttachment, "color_attachment"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eDepthAttachment, "depth_attachment"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eStencilAttachment, "stencil_attachment"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eIndex, "index"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eVertex, "vertex"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eSample, "sample"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eStorage, "storage"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eIndirect, "indirect"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eGeneral, "general"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eCopy, "copy"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::ePresent, "present"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eColorAttachmentRead, "color_attachment_read"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eColorAttachmentWrite, "color_attachment_write"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eDepthAttachmentRead, "depth_attachment_read"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eDepthAttachmentWrite, "depth_attachment_write"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eStencilAttachmentRead, "stencil_attachment_read"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eStencilAttachmentWrite, "stencil_attachment_write"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eDepthStencilAttachmentRead, "depth_stencil_attachment_read"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eDepthStencilAttachmentWrite, "depth_stencil_attachment_write"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eShaderReadOnlyResource, "shader_read_only_resource"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eCopyDst, "copy_dst"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::eCopySrc, "copy_src"),
	EDGE_ENUM_META(edge::gfx::ResourceStateFlag::ePresentSrc, "present_src")
)

EDGE_REGISTER_ENUM_FLAGS(edge::gfx, AccessTypeFlags, AccessTypeFlag,
	EDGE_ENUM_META(edge::gfx::AccessTypeFlag::eRead, "read"),
	EDGE_ENUM_META(edge::gfx::AccessTypeFlag::eWrite, "write"),
	EDGE_ENUM_META(edge::gfx::AccessTypeFlag::eRW, "rw")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, Filter,
	EDGE_ENUM_META(edge::gfx::Filter::eNearest, "nearest"),
	EDGE_ENUM_META(edge::gfx::Filter::eLinear, "linear")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, SamplerAddressMode,
	EDGE_ENUM_META(edge::gfx::SamplerAddressMode::eMirror, "mirror"),
	EDGE_ENUM_META(edge::gfx::SamplerAddressMode::eRepeat, "repeat"),
	EDGE_ENUM_META(edge::gfx::SamplerAddressMode::eClampToEdge, "clamp_to_edge"),
	EDGE_ENUM_META(edge::gfx::SamplerAddressMode::eClampToBorder, "clamp_to_border")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, CompareOp,
	EDGE_ENUM_META(edge::gfx::CompareOp::eNone, "none"),
	EDGE_ENUM_META(edge::gfx::CompareOp::eNever, "never"),
	EDGE_ENUM_META(edge::gfx::CompareOp::eLess, "less"),
	EDGE_ENUM_META(edge::gfx::CompareOp::eEqual, "equal"),
	EDGE_ENUM_META(edge::gfx::CompareOp::eLessOrEqual, "less_or_equal"),
	EDGE_ENUM_META(edge::gfx::CompareOp::eGreater, "greater"),
	EDGE_ENUM_META(edge::gfx::CompareOp::eNotEqual, "not_equal"),
	EDGE_ENUM_META(edge::gfx::CompareOp::eGreaterOrEqual, "greater_or_equal"),
	EDGE_ENUM_META(edge::gfx::CompareOp::eAlways, "always")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, QueryType,
	EDGE_ENUM_META(edge::gfx::QueryType::eTimestamp, "timestamp"),
	EDGE_ENUM_META(edge::gfx::QueryType::eOcclusion, "occlusion"),
	EDGE_ENUM_META(edge::gfx::QueryType::ePipelineStatistics, "pipeline_statistics")
)

EDGE_REGISTER_ENUM_FLAGS(edge::gfx, BufferUsageFlags, BufferUsageFlag,
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eCopy, "copy"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eStorage, "storage"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eVertex, "vertex"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eIndex, "index"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eUniform, "uniform"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eIndirect, "indirect"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eGpuOnly, "gpu_only"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eCpuToGpu, "cpu_to_gpu"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eHasDeviceAddress, "has_device_address"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eStagingBuffer, "staging_buffer"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eStorageBufferCPU, "storage_buffer_cpu"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eStorageBuffer, "storage_buffer"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eVertexBuffer, "vertex_buffer"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eIndexBuffer, "index_buffer"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eUniformBuffer, "uniform_buffer"),
	EDGE_ENUM_META(edge::gfx::BufferUsageFlag::eIndirectBuffer, "indirect_buffer")
)

EDGE_REGISTER_ENUM_FLAGS(edge::gfx, TextureUsageFlags, TextureUsageFlag,
	EDGE_ENUM_META(edge::gfx::TextureUsageFlag::eCubemap, "cubemap"),
	EDGE_ENUM_META(edge::gfx::TextureUsageFlag::eTransferable, "transferable"),
	EDGE_ENUM_META(edge::gfx::TextureUsageFlag::eSamplable, "samplable"),
	EDGE_ENUM_META(edge::gfx::TextureUsageFlag::eStorage, "storage"),
	EDGE_ENUM_META(edge::gfx::TextureUsageFlag::eRenderTarget, "render_target")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, PipelineType,
	EDGE_ENUM_META(edge::gfx::PipelineType::eCompute, "compute"),
	EDGE_ENUM_META(edge::gfx::PipelineType::eGraphics, "graphics")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, PrimitiveTopology,
	EDGE_ENUM_META(edge::gfx::PrimitiveTopology::ePointList, "point_list"),
	EDGE_ENUM_META(edge::gfx::PrimitiveTopology::eLineList, "line_list"),
	EDGE_ENUM_META(edge::gfx::PrimitiveTopology::eLineStrip, "line_strip"),
	EDGE_ENUM_META(edge::gfx::PrimitiveTopology::eTriangleList, "triangle_list"),
	EDGE_ENUM_META(edge::gfx::PrimitiveTopology::eTriangleStrip, "triangle_strip"),
	EDGE_ENUM_META(edge::gfx::PrimitiveTopology::ePatchList, "patch_list")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, SampleCount,
	EDGE_ENUM_META(edge::gfx::SampleCount::e1, "1"),
	EDGE_ENUM_META(edge::gfx::SampleCount::e2, "2"),
	EDGE_ENUM_META(edge::gfx::SampleCount::e4, "4"),
	EDGE_ENUM_META(edge::gfx::SampleCount::e8, "8"),
	EDGE_ENUM_META(edge::gfx::SampleCount::e16, "16")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, FillMode,
	EDGE_ENUM_META(edge::gfx::FillMode::eSolid, "solid"),
	EDGE_ENUM_META(edge::gfx::FillMode::eWireframe, "wireframe")
)

EDGE_REGISTER_ENUM_FLAGS(edge::gfx, CullModeFlags, CullModeFlag,
	EDGE_ENUM_META(edge::gfx::CullModeFlag::eNone, "none"),
	EDGE_ENUM_META(edge::gfx::CullModeFlag::eFront, "front"),
	EDGE_ENUM_META(edge::gfx::CullModeFlag::eBack, "back")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, FrontFace,
	EDGE_ENUM_META(edge::gfx::FrontFace::eCCW, "ccw"),
	EDGE_ENUM_META(edge::gfx::FrontFace::eCW, "cw")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, StencilOp,
	EDGE_ENUM_META(edge::gfx::StencilOp::eKeep, "keep"),
	EDGE_ENUM_META(edge::gfx::StencilOp::eZero, "zero"),
	EDGE_ENUM_META(edge::gfx::StencilOp::eReplace, "replace"),
	EDGE_ENUM_META(edge::gfx::StencilOp::eIncrementAndClamp, "increment_and_clamp"),
	EDGE_ENUM_META(edge::gfx::StencilOp::eDecrementAndClamp, "decrement_and_clamp"),
	EDGE_ENUM_META(edge::gfx::StencilOp::eInvert, "invert"),
	EDGE_ENUM_META(edge::gfx::StencilOp::eIncrementAndWrap, "increment_and_wrap"),
	EDGE_ENUM_META(edge::gfx::StencilOp::eDecrementAndWrap, "decrement_and_wrap")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, BlendFactor,
	EDGE_ENUM_META(edge::gfx::BlendFactor::eZero, "zero"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eOne, "one"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eSrcColor, "src_color"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eOneMinusSrcColor, "one_minus_src_color"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eDstColor, "dst_color"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eOneMinusDstColor, "one_minus_dst_color"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eSrcAlpha, "src_alpha"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eOneMinusSrcAlpha, "one_minus_src_alpha"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eDstAlpha, "dst_alpha"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eOneMinusDstAlpha, "one_minus_dst_alpha"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eConstantAlpha, "constant_alpha"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eOneMinusConstantAlpha, "one_minus_constant_alpha"),
	EDGE_ENUM_META(edge::gfx::BlendFactor::eSrcAlphaSaturate, "src_alpha_saturate")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, BlendOp,
	EDGE_ENUM_META(edge::gfx::BlendOp::eAdd, "add"),
	EDGE_ENUM_META(edge::gfx::BlendOp::eSubtract, "subtract"),
	EDGE_ENUM_META(edge::gfx::BlendOp::eReverseSubtract, "reverse_subtract"),
	EDGE_ENUM_META(edge::gfx::BlendOp::eMin, "min"),
	EDGE_ENUM_META(edge::gfx::BlendOp::eMax, "max")
)

EDGE_REGISTER_ENUM_FLAGS(edge::gfx, ColorMaskFlags, ColorMaskFlag,
	EDGE_ENUM_META(edge::gfx::ColorMaskFlag::eR, "r"),
	EDGE_ENUM_META(edge::gfx::ColorMaskFlag::eG, "g"),
	EDGE_ENUM_META(edge::gfx::ColorMaskFlag::eB, "b"),
	EDGE_ENUM_META(edge::gfx::ColorMaskFlag::eA, "a"),
	EDGE_ENUM_META(edge::gfx::ColorMaskFlag::eAll, "all")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, LoadOp,
	EDGE_ENUM_META(edge::gfx::LoadOp::eLoad, "load"),
	EDGE_ENUM_META(edge::gfx::LoadOp::eClear, "clear"),
	EDGE_ENUM_META(edge::gfx::LoadOp::eDontCare, "dont_care")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, StoreOp,
	EDGE_ENUM_META(edge::gfx::StoreOp::eStore, "store"),
	EDGE_ENUM_META(edge::gfx::StoreOp::eDontCare, "dont_care"),
	EDGE_ENUM_META(edge::gfx::StoreOp::eNone, "none")
)

EDGE_REGISTER_ENUM_CLASS(edge::gfx, DescriptorType,
	EDGE_ENUM_META(edge::gfx::DescriptorType::eSampler, "sampler"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eCombinedImageSampler, "combined_image_sampler"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eSampledImage, "sampled_image"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eStorageImage, "storage_image"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eUniformTexelBuffer, "uniform_texel_buffer"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eStorageTexelBuffer, "storage_texel_buffer"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eUniformBuffer, "uniform_buffer"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eStorageBuffer, "storage_buffer"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eUniformBufferDynamic, "uniform_buffer_dynamic"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eStorageBufferDynamic, "storage_buffer_dynamic"),
	EDGE_ENUM_META(edge::gfx::DescriptorType::eInputAttachment, "input_attachment")
)

namespace edge::gfx
{
	void to_json(nlohmann::json& json, const ShaderStageFlags& type);
	void from_json(const nlohmann::json& json, ShaderStageFlags& type);

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