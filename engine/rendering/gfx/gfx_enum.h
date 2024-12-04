#pragma once

#include <enum.hpp>
#include <nlohmann/json.hpp>

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
UTL_REGISTER_ENUM_CLASS(engine::gfx, RenderApi,
	UTL_ENUM_META(engine::gfx::RenderApi::eAuto, "Auto"),
	UTL_ENUM_META(engine::gfx::RenderApi::eVulkan, "Vulkan"),
	UTL_ENUM_META(engine::gfx::RenderApi::eDirectx12, "Directx12")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx, ResourceType,
	UTL_ENUM_META(engine::gfx::ResourceType::eUnknown, "Auto"),
	UTL_ENUM_META(engine::gfx::ResourceType::eFence, "Fence"),
	UTL_ENUM_META(engine::gfx::ResourceType::eSemaphore, "Semaphore"),
	UTL_ENUM_META(engine::gfx::ResourceType::eQueue, "Queue"),
	UTL_ENUM_META(engine::gfx::ResourceType::eSwapchain, "Swapchain"),
	UTL_ENUM_META(engine::gfx::ResourceType::eBuffer, "Buffer"),
	UTL_ENUM_META(engine::gfx::ResourceType::eCommandList, "CommandList"),
	UTL_ENUM_META(engine::gfx::ResourceType::eCommandPool, "CommandPool"),
	UTL_ENUM_META(engine::gfx::ResourceType::eTexture, "Texture"),
	UTL_ENUM_META(engine::gfx::ResourceType::eSampler, "Sampler"),
	UTL_ENUM_META(engine::gfx::ResourceType::eShader, "Shader"),
	UTL_ENUM_META(engine::gfx::ResourceType::eRootSignature, "RootSignature"),
	UTL_ENUM_META(engine::gfx::ResourceType::ePipeline, "Pipeline"),
	UTL_ENUM_META(engine::gfx::ResourceType::ePipelineCache, "PipelineCache"),
	UTL_ENUM_META(engine::gfx::ResourceType::eResourceDescriptor, "ResourceDescriptor"),
	UTL_ENUM_META(engine::gfx::ResourceType::eDescriptorAllocator, "DescriptorAllocator"),
	UTL_ENUM_META(engine::gfx::ResourceType::eSharedDescriptor, "SharedDescriptor"),
	UTL_ENUM_META(engine::gfx::ResourceType::eQueryPool, "QueryPool")
)

UTL_REGISTER_ENUM_FLAGS(engine::gfx, ShaderStageFlags, ShaderStageFlag,
	UTL_ENUM_META(engine::gfx::ShaderStageFlag::eVertex, "Vertex"),
	UTL_ENUM_META(engine::gfx::ShaderStageFlag::eTessellationControl, "TessellationControl"),
	UTL_ENUM_META(engine::gfx::ShaderStageFlag::eTessellationEvaluation, "TessellationEvaluation"),
	UTL_ENUM_META(engine::gfx::ShaderStageFlag::eGeometry, "Geometry"),
	UTL_ENUM_META(engine::gfx::ShaderStageFlag::eFragment, "Fragment"),
	UTL_ENUM_META(engine::gfx::ShaderStageFlag::eCompute, "Compute"),
	UTL_ENUM_META(engine::gfx::ShaderStageFlag::eAllGraphics, "AllGraphics")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx, IndexType,
	UTL_ENUM_META(engine::gfx::IndexType::eUint16, "Uint16"),
	UTL_ENUM_META(engine::gfx::IndexType::eUint32, "Uint32")
)

UTL_REGISTER_ENUM_FLAGS(engine::gfx, QueueTypeFlags, QueueTypeFlag,
	UTL_ENUM_META(engine::gfx::QueueTypeFlag::eGraphics, "Graphics"),
	UTL_ENUM_META(engine::gfx::QueueTypeFlag::ePresent, "Present"),
	UTL_ENUM_META(engine::gfx::QueueTypeFlag::eCompute, "Compute"),
	UTL_ENUM_META(engine::gfx::QueueTypeFlag::eTransfer, "Transfer")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx, CommandPoolResetMode,
	UTL_ENUM_META(engine::gfx::CommandPoolResetMode::eResetPool, "ResetPool"),
	UTL_ENUM_META(engine::gfx::CommandPoolResetMode::eResetIndividually, "ResetIndividually"),
	UTL_ENUM_META(engine::gfx::CommandPoolResetMode::eAlwaysAllocate, "AlwaysAllocate")
)

UTL_REGISTER_ENUM_FLAGS(engine::gfx, StageFlags, StageFlag,
	UTL_ENUM_META(engine::gfx::StageFlag::eNone, "None"),
	UTL_ENUM_META(engine::gfx::StageFlag::eTopOfPipe, "TopOfPipe"),
	UTL_ENUM_META(engine::gfx::StageFlag::eDrawIndirect, "DrawIndirect"),
	UTL_ENUM_META(engine::gfx::StageFlag::eVertexInput, "VertexInput"),
	UTL_ENUM_META(engine::gfx::StageFlag::eVertexShader, "VertexShader"),
	UTL_ENUM_META(engine::gfx::StageFlag::eTessellationControlShader, "TessellationControlShader"),
	UTL_ENUM_META(engine::gfx::StageFlag::eTessellationEvaluationShader, "TessellationEvaluationShader"),
	UTL_ENUM_META(engine::gfx::StageFlag::eGeometryShader, "GeometryShader"),
	UTL_ENUM_META(engine::gfx::StageFlag::eFragmentShader, "FragmentShader"),
	UTL_ENUM_META(engine::gfx::StageFlag::eEarlyFragmentTests, "EarlyFragmentTests"),
	UTL_ENUM_META(engine::gfx::StageFlag::eLateFragmentTests, "LateFragmentTests"),
	UTL_ENUM_META(engine::gfx::StageFlag::eColorAttachmentOutput, "ColorAttachmentOutput"),
	UTL_ENUM_META(engine::gfx::StageFlag::eComputeShader, "ComputeShader"),
	UTL_ENUM_META(engine::gfx::StageFlag::eAllTransfer, "AllTransfer"),
	UTL_ENUM_META(engine::gfx::StageFlag::eTransfer, "Transfer"),
	UTL_ENUM_META(engine::gfx::StageFlag::eBottomOfPipe, "BottomOfPipe"),
	UTL_ENUM_META(engine::gfx::StageFlag::eHost, "Host"),
	UTL_ENUM_META(engine::gfx::StageFlag::eAllGraphics, "AllGraphics"),
	UTL_ENUM_META(engine::gfx::StageFlag::eAllCommands, "AllCommands"),
	UTL_ENUM_META(engine::gfx::StageFlag::eCopy, "Copy"),
	UTL_ENUM_META(engine::gfx::StageFlag::eResolve, "Resolve"),
	UTL_ENUM_META(engine::gfx::StageFlag::eBlit, "Blit"),
	UTL_ENUM_META(engine::gfx::StageFlag::eClear, "Clear"),
	UTL_ENUM_META(engine::gfx::StageFlag::eIndexInput, "IndexInput"),
	UTL_ENUM_META(engine::gfx::StageFlag::eVertexAttributeInput, "VertexAttributeInput"),
	UTL_ENUM_META(engine::gfx::StageFlag::ePreRasterizationShaders, "PreRasterizationShaders")
)

UTL_REGISTER_ENUM_FLAGS(engine::gfx, ResourceStateFlags, ResourceStateFlag,
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eUndefined, "Undefined"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eRead, "Read"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eWrite, "Write"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eCompute, "Compute"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eGraphics, "Graphics"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eColorAttachment, "ColorAttachment"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eDepthAttachment, "DepthAttachment"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eStencilAttachment, "StencilAttachment"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eSample, "Sample"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eStorage, "Storage"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eIndirect, "Indirect"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eGeneral, "General"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eCopy, "Copy"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::ePresent, "Present"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eColorAttachmentRead, "ColorAttachmentRead"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eColorAttachmentWrite, "ColorAttachmentWrite"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eDepthAttachmentRead, "DepthAttachmentRead"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eDepthAttachmentWrite, "DepthAttachmentWrite"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eStencilAttachmentRead, "StencilAttachmentRead"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eStencilAttachmentWrite, "StencilAttachmentWrite"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eDepthStencilAttachmentRead, "DepthStencilAttachmentRead"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eDepthStencilAttachmentWrite, "DepthStencilAttachmentWrite"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eShaderReadOnlyResource, "ShaderReadOnlyResource"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eCopyDst, "CopyDst"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::eCopySrc, "CopySrc"),
	UTL_ENUM_META(engine::gfx::ResourceStateFlag::ePresentSrc, "PresentSrc")
)

UTL_REGISTER_ENUM_FLAGS(engine::gfx, AccessTypeFlags, AccessTypeFlag,
	UTL_ENUM_META(engine::gfx::AccessTypeFlag::eRead, "Read"),
	UTL_ENUM_META(engine::gfx::AccessTypeFlag::eWrite, "Write"),
	UTL_ENUM_META(engine::gfx::AccessTypeFlag::eRW, "RW")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx, Filter,
	UTL_ENUM_META(engine::gfx::Filter::eNearest, "Nearest"),
	UTL_ENUM_META(engine::gfx::Filter::eLinear, "Linear")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx, SamplerAddressMode,
	UTL_ENUM_META(engine::gfx::SamplerAddressMode::eMirror, "Mirror"),
	UTL_ENUM_META(engine::gfx::SamplerAddressMode::eRepeat, "Repeat"),
	UTL_ENUM_META(engine::gfx::SamplerAddressMode::eClampToEdge, "ClampToEdge"),
	UTL_ENUM_META(engine::gfx::SamplerAddressMode::eClampToBorder, "ClampToBorder")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx, CompareOp,
	UTL_ENUM_META(engine::gfx::CompareOp::eNone, "None"),
	UTL_ENUM_META(engine::gfx::CompareOp::eNever, "Never"),
	UTL_ENUM_META(engine::gfx::CompareOp::eLess, "Less"),
	UTL_ENUM_META(engine::gfx::CompareOp::eEqual, "Equal"),
	UTL_ENUM_META(engine::gfx::CompareOp::eLessOrEqual, "LessOrEqual"),
	UTL_ENUM_META(engine::gfx::CompareOp::eGreater, "Greater"),
	UTL_ENUM_META(engine::gfx::CompareOp::eNotEqual, "NotEqual"),
	UTL_ENUM_META(engine::gfx::CompareOp::eGreaterOrEqual, "GreaterOrEqual"),
	UTL_ENUM_META(engine::gfx::CompareOp::eAlways, "Always")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx, QueryType,
	UTL_ENUM_META(engine::gfx::QueryType::eTimestamp, "Timestamp"),
	UTL_ENUM_META(engine::gfx::QueryType::eOcclusion, "Occlusion"),
	UTL_ENUM_META(engine::gfx::QueryType::ePipelineStatistics, "PipelineStatistics")
)

UTL_REGISTER_ENUM_FLAGS(engine::gfx, BufferUsageFlags, BufferUsageFlag,
	UTL_ENUM_META(engine::gfx::BufferUsageFlag::eCopy, "Copy"),
	UTL_ENUM_META(engine::gfx::BufferUsageFlag::eStorage, "Storage"),
	UTL_ENUM_META(engine::gfx::BufferUsageFlag::eVertex, "Vertex"),
	UTL_ENUM_META(engine::gfx::BufferUsageFlag::eIndex, "Index"),
	UTL_ENUM_META(engine::gfx::BufferUsageFlag::eUniform, "Uniform"),
	UTL_ENUM_META(engine::gfx::BufferUsageFlag::eIndirect, "Indirect"),
	UTL_ENUM_META(engine::gfx::BufferUsageFlag::eGpuOnly, "GpuOnly"),
	UTL_ENUM_META(engine::gfx::BufferUsageFlag::eCpuToGpu, "CpuToGpu"),
	UTL_ENUM_META(engine::gfx::BufferUsageFlag::eHasDeviceAddress, "HasDeviceAddress"),
	UTL_ENUM_META(engine::gfx::BufferUsageFlag::eStagingBuffer, "StagingBuffer"),
	UTL_ENUM_META(engine::gfx::BufferUsageFlag::eStorageBufferCPU, "StorageBufferCPU"),
	UTL_ENUM_META(engine::gfx::BufferUsageFlag::eStorageBuffer, "StorageBuffer"),
	UTL_ENUM_META(engine::gfx::BufferUsageFlag::eVertexBuffer, "VertexBuffer"),
	UTL_ENUM_META(engine::gfx::BufferUsageFlag::eIndexBuffer, "IndexBuffer"),
	UTL_ENUM_META(engine::gfx::BufferUsageFlag::eUniformBuffer, "UniformBuffer"),
	UTL_ENUM_META(engine::gfx::BufferUsageFlag::eIndirectBuffer, "IndirectBuffer")
)

UTL_REGISTER_ENUM_FLAGS(engine::gfx, TextureUsageFlags, TextureUsageFlag,
	UTL_ENUM_META(engine::gfx::TextureUsageFlag::eCubemap, "Cubemap"),
	UTL_ENUM_META(engine::gfx::TextureUsageFlag::eTransferable, "Transferable"),
	UTL_ENUM_META(engine::gfx::TextureUsageFlag::eSamplable, "Samplable"),
	UTL_ENUM_META(engine::gfx::TextureUsageFlag::eStorage, "Storage"),
	UTL_ENUM_META(engine::gfx::TextureUsageFlag::eRenderTarget, "RenderTarget")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx, PipelineType,
	UTL_ENUM_META(engine::gfx::PipelineType::eCompute, "Compute"),
	UTL_ENUM_META(engine::gfx::PipelineType::eGraphics, "Graphics")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx, PrimitiveTopology,
	UTL_ENUM_META(engine::gfx::PrimitiveTopology::ePointList, "PointList"),
	UTL_ENUM_META(engine::gfx::PrimitiveTopology::eLineList, "LineList"),
	UTL_ENUM_META(engine::gfx::PrimitiveTopology::eLineStrip, "LineStrip"),
	UTL_ENUM_META(engine::gfx::PrimitiveTopology::eTriangleList, "TriangleList"),
	UTL_ENUM_META(engine::gfx::PrimitiveTopology::eTriangleStrip, "TriangleStrip"),
	UTL_ENUM_META(engine::gfx::PrimitiveTopology::ePatchList, "PatchList")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx, SampleCount,
	UTL_ENUM_META(engine::gfx::SampleCount::e1, "1"),
	UTL_ENUM_META(engine::gfx::SampleCount::e2, "2"),
	UTL_ENUM_META(engine::gfx::SampleCount::e4, "4"),
	UTL_ENUM_META(engine::gfx::SampleCount::e8, "8"),
	UTL_ENUM_META(engine::gfx::SampleCount::e16, "16")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx, FillMode,
	UTL_ENUM_META(engine::gfx::FillMode::eSolid, "Solid"),
	UTL_ENUM_META(engine::gfx::FillMode::eWireframe, "Wireframe")
)

UTL_REGISTER_ENUM_FLAGS(engine::gfx, CullModeFlags, CullModeFlag,
	UTL_ENUM_META(engine::gfx::CullModeFlag::eNone, "None"),
	UTL_ENUM_META(engine::gfx::CullModeFlag::eFront, "Front"),
	UTL_ENUM_META(engine::gfx::CullModeFlag::eBack, "Back")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx, FrontFace,
	UTL_ENUM_META(engine::gfx::FrontFace::eCCW, "CCW"),
	UTL_ENUM_META(engine::gfx::FrontFace::eCW, "CW")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx, StencilOp,
	UTL_ENUM_META(engine::gfx::StencilOp::eKeep, "Keep"),
	UTL_ENUM_META(engine::gfx::StencilOp::eZero, "Zero"),
	UTL_ENUM_META(engine::gfx::StencilOp::eReplace, "Replace"),
	UTL_ENUM_META(engine::gfx::StencilOp::eIncrementAndClamp, "IncrementAndClamp"),
	UTL_ENUM_META(engine::gfx::StencilOp::eDecrementAndClamp, "DecrementAndClamp"),
	UTL_ENUM_META(engine::gfx::StencilOp::eInvert, "Invert"),
	UTL_ENUM_META(engine::gfx::StencilOp::eIncrementAndWrap, "IncrementAndWrap"),
	UTL_ENUM_META(engine::gfx::StencilOp::eDecrementAndWrap, "DecrementAndWrap")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx, BlendFactor,
	UTL_ENUM_META(engine::gfx::BlendFactor::eZero, "Zero"),
	UTL_ENUM_META(engine::gfx::BlendFactor::eOne, "One"),
	UTL_ENUM_META(engine::gfx::BlendFactor::eSrcColor, "SrcColor"),
	UTL_ENUM_META(engine::gfx::BlendFactor::eOneMinusSrcColor, "OneMinusSrcColor"),
	UTL_ENUM_META(engine::gfx::BlendFactor::eDstColor, "DstColor"),
	UTL_ENUM_META(engine::gfx::BlendFactor::eOneMinusDstColor, "OneMinusDstColor"),
	UTL_ENUM_META(engine::gfx::BlendFactor::eSrcAlpha, "SrcAlpha"),
	UTL_ENUM_META(engine::gfx::BlendFactor::eOneMinusSrcAlpha, "OneMinusSrcAlpha"),
	UTL_ENUM_META(engine::gfx::BlendFactor::eOneMinusSrcAlpha, "OneMinusSrcAlpha"),
	UTL_ENUM_META(engine::gfx::BlendFactor::eDstAlpha, "DstAlpha"),
	UTL_ENUM_META(engine::gfx::BlendFactor::eOneMinusDstAlpha, "OneMinusDstAlpha"),
	UTL_ENUM_META(engine::gfx::BlendFactor::eConstantAlpha, "ConstantAlpha"),
	UTL_ENUM_META(engine::gfx::BlendFactor::eOneMinusConstantAlpha, "OneMinusConstantAlpha"),
	UTL_ENUM_META(engine::gfx::BlendFactor::eSrcAlphaSaturate, "SrcAlphaSaturate")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx, BlendOp,
	UTL_ENUM_META(engine::gfx::BlendOp::eAdd, "Add"),
	UTL_ENUM_META(engine::gfx::BlendOp::eSubtract, "Subtract"),
	UTL_ENUM_META(engine::gfx::BlendOp::eReverseSubtract, "ReverseSubtract"),
	UTL_ENUM_META(engine::gfx::BlendOp::eMin, "Min"),
	UTL_ENUM_META(engine::gfx::BlendOp::eMax, "Max")
)

UTL_REGISTER_ENUM_FLAGS(engine::gfx, ColorMaskFlags, ColorMaskFlag,
	UTL_ENUM_META(engine::gfx::ColorMaskFlag::eR, "R"),
	UTL_ENUM_META(engine::gfx::ColorMaskFlag::eG, "G"),
	UTL_ENUM_META(engine::gfx::ColorMaskFlag::eB, "B"),
	UTL_ENUM_META(engine::gfx::ColorMaskFlag::eA, "A"),
	UTL_ENUM_META(engine::gfx::ColorMaskFlag::eAll, "All")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx, LoadOp,
	UTL_ENUM_META(engine::gfx::LoadOp::eLoad, "Load"),
	UTL_ENUM_META(engine::gfx::LoadOp::eClear, "Clear"),
	UTL_ENUM_META(engine::gfx::LoadOp::eDontCare, "DontCare")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx, StoreOp,
	UTL_ENUM_META(engine::gfx::StoreOp::eStore, "Store"),
	UTL_ENUM_META(engine::gfx::StoreOp::eDontCare, "DontCare"),
	UTL_ENUM_META(engine::gfx::StoreOp::eNone, "None")
)

UTL_REGISTER_ENUM_CLASS(engine::gfx, DescriptorType,
	UTL_ENUM_META(engine::gfx::DescriptorType::eSampler, "Sampler"),
	UTL_ENUM_META(engine::gfx::DescriptorType::eCombinedImageSampler, "CombinedImageSampler"),
	UTL_ENUM_META(engine::gfx::DescriptorType::eSampledImage, "SampledImage"),
	UTL_ENUM_META(engine::gfx::DescriptorType::eStorageImage, "StorageImage"),
	UTL_ENUM_META(engine::gfx::DescriptorType::eUniformTexelBuffer, "UniformTexelBuffer"),
	UTL_ENUM_META(engine::gfx::DescriptorType::eStorageTexelBuffer, "StorageTexelBuffer"),
	UTL_ENUM_META(engine::gfx::DescriptorType::eUniformBuffer, "UniformBuffer"),
	UTL_ENUM_META(engine::gfx::DescriptorType::eStorageBuffer, "StorageBuffer"),
	UTL_ENUM_META(engine::gfx::DescriptorType::eUniformBufferDynamic, "UniformBufferDynamic"),
	UTL_ENUM_META(engine::gfx::DescriptorType::eStorageBufferDynamic, "StorageBufferDynamic"),
	UTL_ENUM_META(engine::gfx::DescriptorType::eInputAttachment, "InputAttachment")
)

namespace engine
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