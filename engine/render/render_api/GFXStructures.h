#pragma once

#include <array>
#include <span>
#include <GFXEnums.h>
#include <GFXFormat.h>

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

	struct GFXContextCreateInfo
	{
		RenderApi render_api{ RenderApi::eVulkan };
		engine::Window* window{ nullptr };
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

	
	// Shader structures
	struct VertexInput
	{
		char name[255];

		uint32_t name_hash{};

		uint32_t location{};

		uint32_t vec_size{};
		uint32_t columns{};
		uint32_t size{};
	};

	struct ShaderResource
	{
		// Resource descriptor type
		DescriptorType type{};

		// Set where resource is located
		uint32_t set{};

		// Resource binding inside set
		uint32_t binding{};

		// Shader stages where resource was used
		ShaderStageFlags stages{};

		// Resource whole size
		uint32_t size{};

		// Name string
		char name[255];

		// Name hash
		uint32_t name_hash{};

		// Range of variable infos
		uint32_t range_begin{ ~0u };
		uint32_t variable_count{};
	};

	struct ShaderVariable
	{
		// Variable name
		char name[255];

		// Variable name hash
		uint32_t name_hash{};

		uint32_t offset{};

		uint32_t size{};
	};

	template<typename _Ty, size_t element_count>
	struct StaticArray
	{
		std::array<_Ty, element_count> data{};
		uint32_t count{};

		_Ty& NewElement()
		{
			assert(count + 1u < element_count);
			return data[count++];
		}

		auto Begin()
		{
			return data.begin();
		}

		auto End()
		{
			assert(count < element_count);
			return data.begin() + count;
		}
	};

	struct ShaderReflection
	{
		StaticArray<VertexInput, 16ull> vertex_inputs{};

		VertexInput& AddVertexInput()
		{
			return vertex_inputs.NewElement();
		}

		// Uniforms/storage blocks/textures/samplers etc.
		StaticArray<ShaderResource, 64ull> shader_resources{};
		
		ShaderResource& AddShaderResource()
		{
			return shader_resources.NewElement();
		}

		// Variables inside uniforms/push constants
		StaticArray<ShaderVariable, 128ull> shader_variables{};

		ShaderVariable& AddShaderVariable()
		{
			return shader_variables.NewElement();
		}

		ShaderResource constants{};

		uint32_t invocation_count{};

		uint32_t output_vertex_count{};

		// Thread group sizes
		std::array<float, 3ull> local_sizes{};

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

	uint32_t Hash(const SamplerCreateInfo& create_info);
}