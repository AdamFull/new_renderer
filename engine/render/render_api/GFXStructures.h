#pragma once

#include <array>
#include <span>
#include <GFXEnums.h>

namespace engine
{
	class Window;
}

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

	struct ShaderStageInfo
	{
		const char* entry_point{ nullptr };
		std::span<uint8_t> code{};
	};

	struct ShaderCreateInfo
	{
		std::array<ShaderStageInfo, 6ull> shader_stages{};
	};

	struct RootSignatureCreateInfo
	{

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
		engine::Window* window{ nullptr };
		Swapchain* old_swapchain{ nullptr };

		uint32_t image_count{ 1u };
		uint32_t width{ 1u };
		uint32_t height{ 1u };

		// TODO: add format and color space selection
		uint32_t format{};
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

		uint32_t width{};
		uint32_t height{};


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
}