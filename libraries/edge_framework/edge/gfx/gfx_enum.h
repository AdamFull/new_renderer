#pragma once

#include <enum.hpp>
#include <nlohmann/json.hpp>

namespace edge
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

		enum class ResourceStateFlag
		{
			eUndefined = 0,
			eRead = 1 << 0,
			eWrite = 1 << 1,
			eCompute = 1 << 2,
			eGraphics = 1 << 3,
			eColorAttachment = 1 << 4,
			eDepthAttachment = 1 << 5,
			eStencilAttachment = 1 << 6,
			eSample = 1 << 7,
			eStorage = 1 << 8,
			eIndirect = 1 << 9,
			eGeneral = 1 << 10,
			eCopy = 1 << 11,
			ePresent = 1 << 12,

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

		enum class AccessTypeFlag
		{
			eRead = 1 << 0,
			eWrite = 1 << 1,
			eRW = eRead | eWrite
		};

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

		enum class TextureUsageFlag
		{
			eCubemap = 1 << 0,
			eTransferable = 1 << 1,
			eSamplable = 1 << 2,
			eStorage = 1 << 3,
			eRenderTarget = 1 << 4
		};

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

// Meta data registration
UTL_REGISTER_ENUM_CLASS(edge::gfx, RenderApi,
	UTL_ENUM_META(edge::gfx::RenderApi::eAuto, "Auto"),
	UTL_ENUM_META(edge::gfx::RenderApi::eVulkan, "Vulkan"),
	UTL_ENUM_META(edge::gfx::RenderApi::eDirectx12, "Directx12")
)

UTL_REGISTER_ENUM_CLASS(edge::gfx, ResourceType,
	UTL_ENUM_META(edge::gfx::ResourceType::eUnknown, "Auto"),
	UTL_ENUM_META(edge::gfx::ResourceType::eFence, "Fence"),
	UTL_ENUM_META(edge::gfx::ResourceType::eSemaphore, "Semaphore"),
	UTL_ENUM_META(edge::gfx::ResourceType::eQueue, "Queue"),
	UTL_ENUM_META(edge::gfx::ResourceType::eSwapchain, "Swapchain"),
	UTL_ENUM_META(edge::gfx::ResourceType::eBuffer, "Buffer"),
	UTL_ENUM_META(edge::gfx::ResourceType::eCommandList, "CommandList"),
	UTL_ENUM_META(edge::gfx::ResourceType::eCommandPool, "CommandPool"),
	UTL_ENUM_META(edge::gfx::ResourceType::eTexture, "Texture"),
	UTL_ENUM_META(edge::gfx::ResourceType::eSampler, "Sampler"),
	UTL_ENUM_META(edge::gfx::ResourceType::eShader, "Shader"),
	UTL_ENUM_META(edge::gfx::ResourceType::eRootSignature, "RootSignature"),
	UTL_ENUM_META(edge::gfx::ResourceType::ePipeline, "Pipeline"),
	UTL_ENUM_META(edge::gfx::ResourceType::ePipelineCache, "PipelineCache"),
	UTL_ENUM_META(edge::gfx::ResourceType::eResourceDescriptor, "ResourceDescriptor"),
	UTL_ENUM_META(edge::gfx::ResourceType::eDescriptorAllocator, "DescriptorAllocator"),
	UTL_ENUM_META(edge::gfx::ResourceType::eSharedDescriptor, "SharedDescriptor"),
	UTL_ENUM_META(edge::gfx::ResourceType::eQueryPool, "QueryPool")
)

UTL_REGISTER_ENUM_FLAGS(edge::gfx, ShaderStageFlags, ShaderStageFlag,
	UTL_ENUM_META(edge::gfx::ShaderStageFlag::eVertex, "Vertex"),
	UTL_ENUM_META(edge::gfx::ShaderStageFlag::eTessellationControl, "TessellationControl"),
	UTL_ENUM_META(edge::gfx::ShaderStageFlag::eTessellationEvaluation, "TessellationEvaluation"),
	UTL_ENUM_META(edge::gfx::ShaderStageFlag::eGeometry, "Geometry"),
	UTL_ENUM_META(edge::gfx::ShaderStageFlag::eFragment, "Fragment"),
	UTL_ENUM_META(edge::gfx::ShaderStageFlag::eCompute, "Compute"),
	UTL_ENUM_META(edge::gfx::ShaderStageFlag::eAllGraphics, "AllGraphics")
)

UTL_REGISTER_ENUM_CLASS(edge::gfx, IndexType,
	UTL_ENUM_META(edge::gfx::IndexType::eUint16, "Uint16"),
	UTL_ENUM_META(edge::gfx::IndexType::eUint32, "Uint32")
)

UTL_REGISTER_ENUM_FLAGS(edge::gfx, QueueTypeFlags, QueueTypeFlag,
	UTL_ENUM_META(edge::gfx::QueueTypeFlag::eGraphics, "Graphics"),
	UTL_ENUM_META(edge::gfx::QueueTypeFlag::ePresent, "Present"),
	UTL_ENUM_META(edge::gfx::QueueTypeFlag::eCompute, "Compute"),
	UTL_ENUM_META(edge::gfx::QueueTypeFlag::eTransfer, "Transfer")
)

UTL_REGISTER_ENUM_CLASS(edge::gfx, CommandPoolResetMode,
	UTL_ENUM_META(edge::gfx::CommandPoolResetMode::eResetPool, "ResetPool"),
	UTL_ENUM_META(edge::gfx::CommandPoolResetMode::eResetIndividually, "ResetIndividually"),
	UTL_ENUM_META(edge::gfx::CommandPoolResetMode::eAlwaysAllocate, "AlwaysAllocate")
)

UTL_REGISTER_ENUM_FLAGS(edge::gfx, StageFlags, StageFlag,
	UTL_ENUM_META(edge::gfx::StageFlag::eNone, "None"),
	UTL_ENUM_META(edge::gfx::StageFlag::eTopOfPipe, "TopOfPipe"),
	UTL_ENUM_META(edge::gfx::StageFlag::eDrawIndirect, "DrawIndirect"),
	UTL_ENUM_META(edge::gfx::StageFlag::eVertexInput, "VertexInput"),
	UTL_ENUM_META(edge::gfx::StageFlag::eVertexShader, "VertexShader"),
	UTL_ENUM_META(edge::gfx::StageFlag::eTessellationControlShader, "TessellationControlShader"),
	UTL_ENUM_META(edge::gfx::StageFlag::eTessellationEvaluationShader, "TessellationEvaluationShader"),
	UTL_ENUM_META(edge::gfx::StageFlag::eGeometryShader, "GeometryShader"),
	UTL_ENUM_META(edge::gfx::StageFlag::eFragmentShader, "FragmentShader"),
	UTL_ENUM_META(edge::gfx::StageFlag::eEarlyFragmentTests, "EarlyFragmentTests"),
	UTL_ENUM_META(edge::gfx::StageFlag::eLateFragmentTests, "LateFragmentTests"),
	UTL_ENUM_META(edge::gfx::StageFlag::eColorAttachmentOutput, "ColorAttachmentOutput"),
	UTL_ENUM_META(edge::gfx::StageFlag::eComputeShader, "ComputeShader"),
	UTL_ENUM_META(edge::gfx::StageFlag::eAllTransfer, "AllTransfer"),
	UTL_ENUM_META(edge::gfx::StageFlag::eTransfer, "Transfer"),
	UTL_ENUM_META(edge::gfx::StageFlag::eBottomOfPipe, "BottomOfPipe"),
	UTL_ENUM_META(edge::gfx::StageFlag::eHost, "Host"),
	UTL_ENUM_META(edge::gfx::StageFlag::eAllGraphics, "AllGraphics"),
	UTL_ENUM_META(edge::gfx::StageFlag::eAllCommands, "AllCommands"),
	UTL_ENUM_META(edge::gfx::StageFlag::eCopy, "Copy"),
	UTL_ENUM_META(edge::gfx::StageFlag::eResolve, "Resolve"),
	UTL_ENUM_META(edge::gfx::StageFlag::eBlit, "Blit"),
	UTL_ENUM_META(edge::gfx::StageFlag::eClear, "Clear"),
	UTL_ENUM_META(edge::gfx::StageFlag::eIndexInput, "IndexInput"),
	UTL_ENUM_META(edge::gfx::StageFlag::eVertexAttributeInput, "VertexAttributeInput"),
	UTL_ENUM_META(edge::gfx::StageFlag::ePreRasterizationShaders, "PreRasterizationShaders")
)

UTL_REGISTER_ENUM_FLAGS(edge::gfx, ResourceStateFlags, ResourceStateFlag,
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eUndefined, "Undefined"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eRead, "Read"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eWrite, "Write"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eCompute, "Compute"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eGraphics, "Graphics"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eColorAttachment, "ColorAttachment"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eDepthAttachment, "DepthAttachment"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eStencilAttachment, "StencilAttachment"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eSample, "Sample"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eStorage, "Storage"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eIndirect, "Indirect"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eGeneral, "General"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eCopy, "Copy"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::ePresent, "Present"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eColorAttachmentRead, "ColorAttachmentRead"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eColorAttachmentWrite, "ColorAttachmentWrite"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eDepthAttachmentRead, "DepthAttachmentRead"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eDepthAttachmentWrite, "DepthAttachmentWrite"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eStencilAttachmentRead, "StencilAttachmentRead"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eStencilAttachmentWrite, "StencilAttachmentWrite"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eDepthStencilAttachmentRead, "DepthStencilAttachmentRead"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eDepthStencilAttachmentWrite, "DepthStencilAttachmentWrite"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eShaderReadOnlyResource, "ShaderReadOnlyResource"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eCopyDst, "CopyDst"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::eCopySrc, "CopySrc"),
	UTL_ENUM_META(edge::gfx::ResourceStateFlag::ePresentSrc, "PresentSrc")
)

UTL_REGISTER_ENUM_FLAGS(edge::gfx, AccessTypeFlags, AccessTypeFlag,
	UTL_ENUM_META(edge::gfx::AccessTypeFlag::eRead, "Read"),
	UTL_ENUM_META(edge::gfx::AccessTypeFlag::eWrite, "Write"),
	UTL_ENUM_META(edge::gfx::AccessTypeFlag::eRW, "RW")
)

UTL_REGISTER_ENUM_CLASS(edge::gfx, Filter,
	UTL_ENUM_META(edge::gfx::Filter::eNearest, "Nearest"),
	UTL_ENUM_META(edge::gfx::Filter::eLinear, "Linear")
)

UTL_REGISTER_ENUM_CLASS(edge::gfx, SamplerAddressMode,
	UTL_ENUM_META(edge::gfx::SamplerAddressMode::eMirror, "Mirror"),
	UTL_ENUM_META(edge::gfx::SamplerAddressMode::eRepeat, "Repeat"),
	UTL_ENUM_META(edge::gfx::SamplerAddressMode::eClampToEdge, "ClampToEdge"),
	UTL_ENUM_META(edge::gfx::SamplerAddressMode::eClampToBorder, "ClampToBorder")
)

UTL_REGISTER_ENUM_CLASS(edge::gfx, CompareOp,
	UTL_ENUM_META(edge::gfx::CompareOp::eNone, "None"),
	UTL_ENUM_META(edge::gfx::CompareOp::eNever, "Never"),
	UTL_ENUM_META(edge::gfx::CompareOp::eLess, "Less"),
	UTL_ENUM_META(edge::gfx::CompareOp::eEqual, "Equal"),
	UTL_ENUM_META(edge::gfx::CompareOp::eLessOrEqual, "LessOrEqual"),
	UTL_ENUM_META(edge::gfx::CompareOp::eGreater, "Greater"),
	UTL_ENUM_META(edge::gfx::CompareOp::eNotEqual, "NotEqual"),
	UTL_ENUM_META(edge::gfx::CompareOp::eGreaterOrEqual, "GreaterOrEqual"),
	UTL_ENUM_META(edge::gfx::CompareOp::eAlways, "Always")
)

UTL_REGISTER_ENUM_CLASS(edge::gfx, QueryType,
	UTL_ENUM_META(edge::gfx::QueryType::eTimestamp, "Timestamp"),
	UTL_ENUM_META(edge::gfx::QueryType::eOcclusion, "Occlusion"),
	UTL_ENUM_META(edge::gfx::QueryType::ePipelineStatistics, "PipelineStatistics")
)

UTL_REGISTER_ENUM_FLAGS(edge::gfx, BufferUsageFlags, BufferUsageFlag,
	UTL_ENUM_META(edge::gfx::BufferUsageFlag::eCopy, "Copy"),
	UTL_ENUM_META(edge::gfx::BufferUsageFlag::eStorage, "Storage"),
	UTL_ENUM_META(edge::gfx::BufferUsageFlag::eVertex, "Vertex"),
	UTL_ENUM_META(edge::gfx::BufferUsageFlag::eIndex, "Index"),
	UTL_ENUM_META(edge::gfx::BufferUsageFlag::eUniform, "Uniform"),
	UTL_ENUM_META(edge::gfx::BufferUsageFlag::eIndirect, "Indirect"),
	UTL_ENUM_META(edge::gfx::BufferUsageFlag::eGpuOnly, "GpuOnly"),
	UTL_ENUM_META(edge::gfx::BufferUsageFlag::eCpuToGpu, "CpuToGpu"),
	UTL_ENUM_META(edge::gfx::BufferUsageFlag::eHasDeviceAddress, "HasDeviceAddress"),
	UTL_ENUM_META(edge::gfx::BufferUsageFlag::eStagingBuffer, "StagingBuffer"),
	UTL_ENUM_META(edge::gfx::BufferUsageFlag::eStorageBufferCPU, "StorageBufferCPU"),
	UTL_ENUM_META(edge::gfx::BufferUsageFlag::eStorageBuffer, "StorageBuffer"),
	UTL_ENUM_META(edge::gfx::BufferUsageFlag::eVertexBuffer, "VertexBuffer"),
	UTL_ENUM_META(edge::gfx::BufferUsageFlag::eIndexBuffer, "IndexBuffer"),
	UTL_ENUM_META(edge::gfx::BufferUsageFlag::eUniformBuffer, "UniformBuffer"),
	UTL_ENUM_META(edge::gfx::BufferUsageFlag::eIndirectBuffer, "IndirectBuffer")
)

UTL_REGISTER_ENUM_FLAGS(edge::gfx, TextureUsageFlags, TextureUsageFlag,
	UTL_ENUM_META(edge::gfx::TextureUsageFlag::eCubemap, "Cubemap"),
	UTL_ENUM_META(edge::gfx::TextureUsageFlag::eTransferable, "Transferable"),
	UTL_ENUM_META(edge::gfx::TextureUsageFlag::eSamplable, "Samplable"),
	UTL_ENUM_META(edge::gfx::TextureUsageFlag::eStorage, "Storage"),
	UTL_ENUM_META(edge::gfx::TextureUsageFlag::eRenderTarget, "RenderTarget")
)

UTL_REGISTER_ENUM_CLASS(edge::gfx, PipelineType,
	UTL_ENUM_META(edge::gfx::PipelineType::eCompute, "Compute"),
	UTL_ENUM_META(edge::gfx::PipelineType::eGraphics, "Graphics")
)

UTL_REGISTER_ENUM_CLASS(edge::gfx, PrimitiveTopology,
	UTL_ENUM_META(edge::gfx::PrimitiveTopology::ePointList, "PointList"),
	UTL_ENUM_META(edge::gfx::PrimitiveTopology::eLineList, "LineList"),
	UTL_ENUM_META(edge::gfx::PrimitiveTopology::eLineStrip, "LineStrip"),
	UTL_ENUM_META(edge::gfx::PrimitiveTopology::eTriangleList, "TriangleList"),
	UTL_ENUM_META(edge::gfx::PrimitiveTopology::eTriangleStrip, "TriangleStrip"),
	UTL_ENUM_META(edge::gfx::PrimitiveTopology::ePatchList, "PatchList")
)

UTL_REGISTER_ENUM_CLASS(edge::gfx, SampleCount,
	UTL_ENUM_META(edge::gfx::SampleCount::e1, "1"),
	UTL_ENUM_META(edge::gfx::SampleCount::e2, "2"),
	UTL_ENUM_META(edge::gfx::SampleCount::e4, "4"),
	UTL_ENUM_META(edge::gfx::SampleCount::e8, "8"),
	UTL_ENUM_META(edge::gfx::SampleCount::e16, "16")
)

UTL_REGISTER_ENUM_CLASS(edge::gfx, FillMode,
	UTL_ENUM_META(edge::gfx::FillMode::eSolid, "Solid"),
	UTL_ENUM_META(edge::gfx::FillMode::eWireframe, "Wireframe")
)

UTL_REGISTER_ENUM_FLAGS(edge::gfx, CullModeFlags, CullModeFlag,
	UTL_ENUM_META(edge::gfx::CullModeFlag::eNone, "None"),
	UTL_ENUM_META(edge::gfx::CullModeFlag::eFront, "Front"),
	UTL_ENUM_META(edge::gfx::CullModeFlag::eBack, "Back")
)

UTL_REGISTER_ENUM_CLASS(edge::gfx, FrontFace,
	UTL_ENUM_META(edge::gfx::FrontFace::eCCW, "CCW"),
	UTL_ENUM_META(edge::gfx::FrontFace::eCW, "CW")
)

UTL_REGISTER_ENUM_CLASS(edge::gfx, StencilOp,
	UTL_ENUM_META(edge::gfx::StencilOp::eKeep, "Keep"),
	UTL_ENUM_META(edge::gfx::StencilOp::eZero, "Zero"),
	UTL_ENUM_META(edge::gfx::StencilOp::eReplace, "Replace"),
	UTL_ENUM_META(edge::gfx::StencilOp::eIncrementAndClamp, "IncrementAndClamp"),
	UTL_ENUM_META(edge::gfx::StencilOp::eDecrementAndClamp, "DecrementAndClamp"),
	UTL_ENUM_META(edge::gfx::StencilOp::eInvert, "Invert"),
	UTL_ENUM_META(edge::gfx::StencilOp::eIncrementAndWrap, "IncrementAndWrap"),
	UTL_ENUM_META(edge::gfx::StencilOp::eDecrementAndWrap, "DecrementAndWrap")
)

UTL_REGISTER_ENUM_CLASS(edge::gfx, BlendFactor,
	UTL_ENUM_META(edge::gfx::BlendFactor::eZero, "Zero"),
	UTL_ENUM_META(edge::gfx::BlendFactor::eOne, "One"),
	UTL_ENUM_META(edge::gfx::BlendFactor::eSrcColor, "SrcColor"),
	UTL_ENUM_META(edge::gfx::BlendFactor::eOneMinusSrcColor, "OneMinusSrcColor"),
	UTL_ENUM_META(edge::gfx::BlendFactor::eDstColor, "DstColor"),
	UTL_ENUM_META(edge::gfx::BlendFactor::eOneMinusDstColor, "OneMinusDstColor"),
	UTL_ENUM_META(edge::gfx::BlendFactor::eSrcAlpha, "SrcAlpha"),
	UTL_ENUM_META(edge::gfx::BlendFactor::eOneMinusSrcAlpha, "OneMinusSrcAlpha"),
	UTL_ENUM_META(edge::gfx::BlendFactor::eOneMinusSrcAlpha, "OneMinusSrcAlpha"),
	UTL_ENUM_META(edge::gfx::BlendFactor::eDstAlpha, "DstAlpha"),
	UTL_ENUM_META(edge::gfx::BlendFactor::eOneMinusDstAlpha, "OneMinusDstAlpha"),
	UTL_ENUM_META(edge::gfx::BlendFactor::eConstantAlpha, "ConstantAlpha"),
	UTL_ENUM_META(edge::gfx::BlendFactor::eOneMinusConstantAlpha, "OneMinusConstantAlpha"),
	UTL_ENUM_META(edge::gfx::BlendFactor::eSrcAlphaSaturate, "SrcAlphaSaturate")
)

UTL_REGISTER_ENUM_CLASS(edge::gfx, BlendOp,
	UTL_ENUM_META(edge::gfx::BlendOp::eAdd, "Add"),
	UTL_ENUM_META(edge::gfx::BlendOp::eSubtract, "Subtract"),
	UTL_ENUM_META(edge::gfx::BlendOp::eReverseSubtract, "ReverseSubtract"),
	UTL_ENUM_META(edge::gfx::BlendOp::eMin, "Min"),
	UTL_ENUM_META(edge::gfx::BlendOp::eMax, "Max")
)

UTL_REGISTER_ENUM_FLAGS(edge::gfx, ColorMaskFlags, ColorMaskFlag,
	UTL_ENUM_META(edge::gfx::ColorMaskFlag::eR, "R"),
	UTL_ENUM_META(edge::gfx::ColorMaskFlag::eG, "G"),
	UTL_ENUM_META(edge::gfx::ColorMaskFlag::eB, "B"),
	UTL_ENUM_META(edge::gfx::ColorMaskFlag::eA, "A"),
	UTL_ENUM_META(edge::gfx::ColorMaskFlag::eAll, "All")
)

UTL_REGISTER_ENUM_CLASS(edge::gfx, LoadOp,
	UTL_ENUM_META(edge::gfx::LoadOp::eLoad, "Load"),
	UTL_ENUM_META(edge::gfx::LoadOp::eClear, "Clear"),
	UTL_ENUM_META(edge::gfx::LoadOp::eDontCare, "DontCare")
)

UTL_REGISTER_ENUM_CLASS(edge::gfx, StoreOp,
	UTL_ENUM_META(edge::gfx::StoreOp::eStore, "Store"),
	UTL_ENUM_META(edge::gfx::StoreOp::eDontCare, "DontCare"),
	UTL_ENUM_META(edge::gfx::StoreOp::eNone, "None")
)

UTL_REGISTER_ENUM_CLASS(edge::gfx, DescriptorType,
	UTL_ENUM_META(edge::gfx::DescriptorType::eSampler, "Sampler"),
	UTL_ENUM_META(edge::gfx::DescriptorType::eCombinedImageSampler, "CombinedImageSampler"),
	UTL_ENUM_META(edge::gfx::DescriptorType::eSampledImage, "SampledImage"),
	UTL_ENUM_META(edge::gfx::DescriptorType::eStorageImage, "StorageImage"),
	UTL_ENUM_META(edge::gfx::DescriptorType::eUniformTexelBuffer, "UniformTexelBuffer"),
	UTL_ENUM_META(edge::gfx::DescriptorType::eStorageTexelBuffer, "StorageTexelBuffer"),
	UTL_ENUM_META(edge::gfx::DescriptorType::eUniformBuffer, "UniformBuffer"),
	UTL_ENUM_META(edge::gfx::DescriptorType::eStorageBuffer, "StorageBuffer"),
	UTL_ENUM_META(edge::gfx::DescriptorType::eUniformBufferDynamic, "UniformBufferDynamic"),
	UTL_ENUM_META(edge::gfx::DescriptorType::eStorageBufferDynamic, "StorageBufferDynamic"),
	UTL_ENUM_META(edge::gfx::DescriptorType::eInputAttachment, "InputAttachment")
)

namespace edge
{
	namespace gfx
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
}