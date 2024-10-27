#pragma once

#include <array>
#include <span>
#include <engine/rendering/gfx/Reflection.h>
#include <engine/rendering/gfx/Format.h>

#ifdef MemoryBarrier
#undef MemoryBarrier
#endif

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

		/**
		* Base structures
		*/
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

		// Vertex input
		struct VertexAttributeDescription
		{
			uint32_t location{ 0u };
			uint32_t binding{ 0u };
			Format format{ Format::eUndefined };
			uint32_t offset{ 0u };
		};

		struct VertexBindingDescription
		{
			uint32_t binding{ 0u };
			uint32_t stride{ 0u };
		};

		struct VertexLayout
		{
			std::array<VertexBindingDescription, 16ull> bindings{};
			uint32_t binding_count{ 0u };
			std::array<VertexAttributeDescription, 16ull> attributes{};
			uint32_t attribute_count{ 0u };
		};

		struct RasterizationState
		{
			bool depth_clamp{ false };
			bool discard_enable{ false };
			FillMode fill_mode{ FillMode::eSolid };
			CullModeFlags cull_mode{ CullModeFlag::eNone };
			FrontFace front_face{ FrontFace::eCCW };
		};

		struct MultisampleState
		{
			SampleCount sample_count{ SampleCount::e1 };
			bool alpha_to_coverage{ false };
			bool alpha_to_one{ false };
		};

		struct DepthStencilState
		{
			bool depth_test{ false };
			bool depth_write{ false };
			CompareOp depth_compare_op{ CompareOp::eNever };

			bool stencil_test{ false };
			CompareOp stencil_front_compare_op{ CompareOp::eNever };
			StencilOp front_fail{ StencilOp::eKeep };
			StencilOp depth_front_fail{ StencilOp::eKeep };
			StencilOp front_pass{ StencilOp::eKeep };

			CompareOp stencil_back_compare_op{ CompareOp::eNever };
			StencilOp back_fail{ StencilOp::eKeep };
			StencilOp depth_back_fail{ StencilOp::eKeep };
			StencilOp back_pass{ StencilOp::eKeep };
		};

		struct ColorBlendAttachment
		{
			bool enable{ false };
			BlendFactor src_color{ BlendFactor::eOne };
			BlendFactor dst_color{ BlendFactor::eZero };
			BlendOp color_blend_op{ BlendOp::eAdd };
			BlendFactor src_alpha{ BlendFactor::eOne };
			BlendFactor dst_alpha{ BlendFactor::eZero };
			BlendOp alpha_blend_op{ BlendOp::eAdd };
			ColorMaskFlags color_mask{ ColorMaskFlag::eAll };
		};

		struct ColorBlendingState
		{
			std::array<ColorBlendAttachment, 8ull> attachments{};
			uint32_t attachment_count{ 0u };
		};

		struct PipelineState
		{
			VertexLayout vertex_layout{};
			RasterizationState rasterization_state{};
			MultisampleState multisample_state{};
			DepthStencilState depth_stencil_state{};
			ColorBlendingState color_blending_state{};

			std::array<Format, 8ull> color_formats{};
			uint32_t color_format_count{};

			Format depth_format{ Format::eUndefined };
			Format stencil_format{ Format::eUndefined };

			PrimitiveTopology primitive_topology{ PrimitiveTopology::ePointList };
		};

		struct GraphicsPipelineCreateInfo
		{
			Shader* shader{ nullptr };
			RootSignature* root_signature{ nullptr };
			PipelineCache* pipeline_cache{ nullptr };

			PipelineState pipeline_state{};
		};

		struct ComputePipelineCreateInfo
		{
			Shader* shader{ nullptr };
			RootSignature* root_signature{ nullptr };
			PipelineCache* pipeline_cache{ nullptr };
		};

		struct ClearColor
		{
			ClearColor() = default;

			ClearColor(std::array<float, 4ull>&& clear_color) :
				color{ std::move(clear_color) }
			{}

			ClearColor(float depth) :
				depth{ depth }
			{}

			ClearColor(uint32_t stencil) :
				stencil{ stencil }
			{}

			std::array<float, 4ull> color{0.f, 0.f, 0.f, 1.f};

			float depth{ 0.f };
			uint32_t stencil{ 0u };
		};

		struct Attachment
		{
			Texture* texture{ nullptr };

			LoadOp load_op{ LoadOp::eLoad };
			StoreOp store_op{ StoreOp::eStore };

			ClearColor clear_color{};
		};

		struct RenderingInfo
		{
			int32_t offset_x{ 0u };
			int32_t offset_y{ 0u };

			uint32_t width{ 1u };
			uint32_t height{ 1u };

			uint32_t layer_count{ 1u };

			std::array<Attachment, 8ull> attachments{};
			uint32_t attachment_count{ 0u };

			Attachment depth_attachment{};
			Attachment stencil_attachment{};
		};

		struct PipelineCacheCreateInfo
		{
			std::span<uint8_t> data{};
		};

		struct SwapChainCreateInfo
		{
			const Window* window{ nullptr };
			Swapchain* old_swapchain{ nullptr };

			uint32_t image_count{ 1u };
			uint32_t width{ 1u };
			uint32_t height{ 1u };

			// TODO: add colorspace selection
			Format format{ Format::eUndefined };
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

			Format format{ Format::eUndefined };
		};

		struct SamplerCreateInfo
		{
			Filter min_filter{ Filter::eNearest };
			Filter mag_filter{ Filter::eNearest };
			Filter mip_map_mode{ Filter::eNearest };
			SamplerAddressMode address_u{ SamplerAddressMode::eRepeat };
			SamplerAddressMode address_v{ SamplerAddressMode::eRepeat };
			SamplerAddressMode address_w{ SamplerAddressMode::eRepeat };
			float mip_lod_bias{ 0.f };
			float min_lod{ 0.f };
			float max_lod{ 1000.f };
			float max_anisotropy{ 0.f };
			CompareOp depth_compare{ CompareOp::eNone };
		};

		struct QueryPoolCreateInfo
		{
			QueryType type{};
			uint32_t query_count{ 0u };
		};

		// Resource barriers
		struct ResourceState
		{
			AccessFlags access{};
			StageFlags stage{};
			const Queue* queue{ nullptr };
		};

		struct TextureAccess
		{
			AccessTypeFlags type{};
			TextureLayout layout{};
		};

		struct BufferRangeInfo
		{
			uint64_t size{ 0ull };
			uint64_t offset{ 0ull };
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
			uint32_t subresource_index{ ~0u };

			TextureAccess src_access{};
			//ResourceState src_state{};
			TextureAccess dst_access{};
			//ResourceState dst_state{};
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

		struct IndirectDrawCommand
		{
			bool indexed_draw{ false };

			Buffer* argument_buffer{ nullptr };
			uint64_t argument_offset{ 0ull };
			uint32_t draw_count{ 0u };
			uint32_t command_stride{ 0u };

			Buffer* count_buffer{ nullptr };
			uint64_t count_offset{ 0ull };
		};

		struct BufferCopyRegion
		{
			uint64_t src_offset{ 0ull };
			uint64_t dst_offset{ 0ull };
			uint64_t size{ 0ull };
		};

		struct CopyBufferInfo
		{
			Buffer* src_buffer{ nullptr };
			Buffer* dst_buffer{ nullptr };

			uint32_t region_count{ 0u };
			std::array<BufferCopyRegion, 64ull> regions{};
		};

		struct TextureCopyRegion
		{
			uint64_t buffer_offset{};

			uint32_t offset_x{ 0u };
			uint32_t offset_y{ 0u };
			uint32_t offset_z{ 0u };

			uint32_t size_x{ 1u };
			uint32_t size_y{ 1u };
			uint32_t size_z{ 1u };

			uint32_t layer{ 0u };
			uint32_t layer_count{ 1u };

			uint32_t level{ 0u };
		};

		struct CopyTextureInfo
		{
			Buffer* src_buffer{ nullptr };
			Texture* dst_texture{ nullptr };

			std::array<TextureCopyRegion, 64ull> copy_regions{};
			uint32_t copy_region_count{ 0u };
		};

		uint32_t Hash(const VertexAttributeDescription& desc, uint32_t hash = 0u);
		uint32_t Hash(const VertexBindingDescription& desc, uint32_t hash = 0u);
		uint32_t Hash(const VertexLayout& layout, uint32_t hash = 0u);
		uint32_t Hash(const RasterizationState& state, uint32_t hash = 0u);
		uint32_t Hash(const MultisampleState& state, uint32_t hash = 0u);
		uint32_t Hash(const DepthStencilState& state, uint32_t hash = 0u);
		uint32_t Hash(const ClearColor& color, uint32_t hash = 0u);
		uint32_t Hash(const ColorBlendAttachment& attachment, uint32_t hash = 0u);
		uint32_t Hash(const ColorBlendingState& state, uint32_t hash = 0u);
		uint32_t Hash(const PipelineState& state, uint32_t hash = 0u);

		uint32_t Hash(const SamplerCreateInfo& create_info, uint32_t hash = 0u);
	}
}