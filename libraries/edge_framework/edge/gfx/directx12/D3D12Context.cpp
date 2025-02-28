#include "D3D12Context.h"

namespace edge
{
	namespace gfx
	{
		namespace d3d12
		{
			D3D12Context::D3D12Context(const ContextCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
			}

			D3D12Context::~D3D12Context()
			{

			}

			void D3D12Context::Destroy(IGFXResource* handle) const
			{
				assert(false && "NOT IMPLEMENTED");
			}

			auto D3D12Context::CreateFence() const -> IFence*
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			auto D3D12Context::CreateSemaphore() const -> ISemaphore*
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			auto D3D12Context::RequestQueue(QueueTypeFlags include_flags, QueueTypeFlags exclude_flags) const -> IQueue*
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			auto D3D12Context::CreateSwapchain(const SwapChainCreateInfo& create_info) const -> ISwapchain*
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			auto D3D12Context::CreateCommandPool(const IQueue* queue, CommandPoolResetMode reset_mode) const -> ICommandPool*
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			auto D3D12Context::CreateBuffer(const BufferCreateInfo& create_info) const -> IBuffer*
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			auto D3D12Context::CreateTexture(const TextureCreateInfo& create_info) const -> ITexture*
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			auto D3D12Context::CreateSwapchainTexture(ISwapchain* swapchain, uint32_t image_index) const -> ITexture*
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			auto D3D12Context::CreateShader(std::span<const ShaderStageInfo> shader_stages) const -> IShader*
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			auto D3D12Context::CreateRootSignature(const RootSignatureCreateInfo& create_info) const -> IRootSignature*
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			auto D3D12Context::CreatePipeline(const GraphicsPipelineCreateInfo& create_info) const -> IPipeline*
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			auto D3D12Context::CreatePipeline(const ComputePipelineCreateInfo& create_info) const -> IPipeline*
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			auto D3D12Context::CreatePipelineCache(const PipelineCacheCreateInfo& create_info) const -> IPipelineCache*
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			auto D3D12Context::CreateDescriptorAllocator() const -> IDescriptorAllocator*
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			auto D3D12Context::CreateSharedDescriptor(const SharedDescriptorCreateInfo& create_info) const -> ISharedDescriptor*
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			auto D3D12Context::CreateSampler(const SamplerCreateInfo& create_info) const -> ISampler*
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			auto D3D12Context::CreateQueryPool(const QueryPoolCreateInfo& create_info) const -> IQueryPool*
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			void D3D12Context::WaitForFences(std::span<IFence*> fences, uint64_t timeout) const
			{
				assert(false && "NOT IMPLEMENTED");
			}

			void D3D12Context::RequestMinimalBufferAlignment(BufferUsageFlags usage, uint64_t& alignment) const
			{
				assert(false && "NOT IMPLEMENTED");
			}

			void D3D12Context::RequestTimestampFrequency(float* frequency) const
			{
				assert(false && "NOT IMPLEMENTED");
			}

			void D3D12Context::RequestMemoryUsage(uint64_t* total_memory, uint64_t* used_memory) const
			{
				assert(false && "NOT IMPLEMENTED");
			}
		}
	}
}