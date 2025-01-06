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

			std::unique_ptr<Fence> D3D12Context::CreateFence()
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			std::unique_ptr<Semaphore> D3D12Context::CreateSemaphore()
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			const Queue* D3D12Context::RequestQueue(QueueTypeFlags queue_type_flags, uint32_t queue_index)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			std::unique_ptr<Swapchain> D3D12Context::CreateSwapchain(const SwapChainCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			std::unique_ptr<CommandPool> D3D12Context::CreateCommandPool(const Queue* queue, CommandPoolResetMode reset_mode)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			std::unique_ptr<Buffer> D3D12Context::CreateBuffer(const BufferCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			std::unique_ptr<Texture> D3D12Context::CreateTexture(const TextureCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			std::unique_ptr<Texture> D3D12Context::CreateSwapchainTexture(Swapchain* swapchain, uint32_t image_index)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			std::unique_ptr<Shader> D3D12Context::CreateShader(std::span<const ShaderStageInfo> shader_stages)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			std::unique_ptr<RootSignature> D3D12Context::CreateRootSignature(const RootSignatureCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			std::unique_ptr<Pipeline> D3D12Context::CreatePipeline(const GraphicsPipelineCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			std::unique_ptr<Pipeline> D3D12Context::CreatePipeline(const ComputePipelineCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			std::unique_ptr<PipelineCache> D3D12Context::CreatePipelineCache(const PipelineCacheCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			std::unique_ptr<DescriptorAllocator> D3D12Context::CreateDescriptorAllocator()
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			std::unique_ptr<SharedDescriptor> D3D12Context::CreateSharedDescriptor(const SharedDescriptorCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			std::unique_ptr<Sampler> D3D12Context::CreateSampler(const SamplerCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			std::unique_ptr<QueryPool> D3D12Context::CreateQueryPool(const QueryPoolCreateInfo& create_info)
			{
				assert(false && "NOT IMPLEMENTED");
				return nullptr;
			}

			void D3D12Context::WaitForFences(std::span<Fence*> fences, uint64_t timeout)
			{
				assert(false && "NOT IMPLEMENTED");
			}

			void D3D12Context::RequestMinimalBufferAlignment(BufferUsageFlags usage, uint64_t& alignment)
			{
				assert(false && "NOT IMPLEMENTED");
			}

			void D3D12Context::RequestTimestampFrequency(float* frequency)
			{
				assert(false && "NOT IMPLEMENTED");
			}

			void D3D12Context::RequestMemoryUsage(uint64_t* total_memory, uint64_t* used_memory)
			{
				assert(false && "NOT IMPLEMENTED");
			}
		}
	}
}