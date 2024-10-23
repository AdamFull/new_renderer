#pragma once

#include <array>
#include <span>
#include <GFXReflection.h>
#include <GFXFormat.h>

namespace engine
{
	class Window;

	namespace gfx
	{
		class GFXResource;
		class Fence;
		class Semaphore;
		class Queue;
		class Swapchain;
		class Buffer;
		class CommandList;
		class CommandPool;
		class Texture;
		class Sampler;
		class Shader;
		class RootSignature;
		class Pipeline;
		class PipelineCache;
		class DescriptorPool;
		class DescriptorSet;
		class QueryPool;

		struct ContextCreateInfo
		{
			RenderApi render_api{ RenderApi::eVulkan };
			Window* window{ nullptr };
			const char* app_name{ nullptr };
		};

		struct SemaphoreSubmitInfo
		{
			Semaphore* semaphore{ nullptr };
			uint64_t value{ 0ull };
			StageFlags stage;
		};

		struct SubmitInfo
		{
			uint32_t wait_semaphore_count{ 0u };
			SemaphoreSubmitInfo* wait_semaphore_infos{ nullptr };
			uint32_t command_buffer_count{ 0u };
			CommandList** command_lists{ nullptr };
			uint32_t signal_semaphore_count{ 0u };
			SemaphoreSubmitInfo* signal_semaphore_infos{ nullptr };
		};

		struct PresentInfo
		{
			uint32_t wait_semaphore_count{ 0u };
			Semaphore** wait_semaphores{ nullptr };
			uint32_t swapchain_count{ 0u };
			Swapchain** swapchains{ nullptr };
			const uint32_t* image_indices{ nullptr };
		};

		struct ShaderStageInfo
		{
			ShaderStageFlag stage{};
			const char* entry_point{ "main" };
			std::span<uint8_t> code{};
		};

		struct ShaderCreateInfo
		{
			ShaderStageFlags stages{};

			// At this moment only 6 shader types is avaliable
			std::array<ShaderStageInfo, 6ull> shader_stages{};
		};

		struct RootSignatureCreateInfo
		{
			Shader* shader{ nullptr };
			Sampler** immutable_samplers{ nullptr };
			uint32_t immutable_sampler_count{};
		};

		struct GraphicsPipelineCreateInfo
		{
			Shader* shader{ nullptr };
			RootSignature* root_signature{ nullptr };
		};

		struct ComputePipelineCreateInfo
		{
			Shader* shader{ nullptr };
			RootSignature* root_signature{ nullptr };
		};

		struct PipelineCacheCreateInfo
		{
			std::span<uint8_t> data{};
		};

		struct SwapChainCreateInfo
		{
			Window* window{ nullptr };
			Swapchain* old_swapchain{ nullptr };

			uint32_t image_count{ 1u };
			uint32_t width{ 1u };
			uint32_t height{ 1u };

			// TODO: add colorspace selection
			Format format{};
		};

		struct BufferCreateInfo
		{
			BufferUsageFlags usage{};
			size_t size{ 1ull };
			size_t count{ 1ull };
		};

		struct TextureCreateInfo
		{
			TextureUsageFlags usage{};

			uint32_t width{ 1u };
			uint32_t height{ 1u };
			uint32_t depth{ 1u };

			uint32_t layers{ 1u };
			uint32_t levels{ 1u };

			Format format{};
		};

		struct SamplerCreateInfo
		{
			Filter min_filter{ Filter::eNearest };
			Filter mag_filter{ Filter::eNearest };
			SamplerMipmapMode  mip_map_mode{ SamplerMipmapMode::eNearest };
			SamplerAddressMode address_u{ SamplerAddressMode::eRepeat };
			SamplerAddressMode address_v{ SamplerAddressMode::eRepeat };
			SamplerAddressMode address_w{ SamplerAddressMode::eRepeat };
			float mip_lod_bias{ 0.f };
			float min_lod{ 0.f };
			float max_lod{ 1.f };
			float max_anisotropy{ 0.f };
			CompareOp depth_compare{ CompareOp::eNone };
		};

		struct QueryPoolCreateInfo
		{
			QueryType type{};
			uint32_t query_count{};
		};

		// Resource barriers
		struct ResourceState
		{
			AccessFlags access{};
			StageFlags stage{};
		};

		struct BufferRangeInfo
		{
			uint64_t size{};
			uint64_t offset{};
		};

		struct BufferBarrier
		{
			Buffer* buffer{ nullptr };
			ResourceState src_state{};
			ResourceState dst_state{};
			BufferRangeInfo range{};
		};

		struct TextureBarrier
		{
			Texture* texture{ nullptr };
		};

		struct MemoryBarrier
		{
			ResourceState src_state{};
			ResourceState dst_state{};
		};

		struct ResourceBarrierInfo
		{
			uint32_t buffer_barrier_count{};
			BufferBarrier* buffer_barriers{ nullptr };

			uint32_t texture_barrier_count{};
			TextureBarrier* texture_barriers{ nullptr };

			uint32_t memory_barrier_count{};
			MemoryBarrier* memory_barriers{ nullptr };
		};

		uint32_t Hash(const SamplerCreateInfo& create_info);
	}
}