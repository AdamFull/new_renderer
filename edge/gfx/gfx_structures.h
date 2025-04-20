#pragma once

#include <atomic>
#include <array>
#include <span>
#include "gfx_shader_reflection.h"
#include "gfx_format.h"

namespace edge
{
	class Window;

	namespace gfx
	{
		static constexpr uint32_t k_max_descriptor_count{ 8u };
		static constexpr uint32_t k_max_attachment_count{ 8u };

		class IGFXResource;
		class IFence;
		class ISemaphore;
		class IQueue;
		class ISwapchain;
		class IBuffer;
		class ICommandList;
		class ICommandPool;
		class ITexture;
		class ISampler;
		class IShader;
		class IRootSignature;
		class IPipeline;
		class IPipelineCache;
		class IDescriptorLayout;
		class IBindingLayout;
		class IDescriptorAllocator;
		class IQueryPool;

		/**
		* Base structures
		*/
		struct ContextCreateInfo {
			RenderApi render_api{ RenderApi::eVulkan };
			Window* window{ nullptr };
			const char* app_name{ nullptr };
		};

		struct SemaphoreSubmitInfo {
			ISemaphore* semaphore{ nullptr };
			uint64_t value{ 0ull };
			StageFlags stage;
		};

		struct SubmitInfo {
			std::span<SemaphoreSubmitInfo> wait_semaphore_infos;
			std::span<ICommandList*> command_lists;
			std::span<SemaphoreSubmitInfo> signal_semaphore_infos;
		};

		struct PresentInfo {
			std::span<ISemaphore*> wait_semaphores;
			std::span<ISwapchain*> swapchains;
			const uint32_t* image_indices{ nullptr };
		};

		struct ShaderStageInfo {
			ShaderStageFlag stage{};
			const char* entry_point{ "main" };
			std::span<const uint8_t> code{};
		};

		struct RootSignatureCreateInfo {
			IShader* shader{ nullptr };
			std::span<IDescriptorLayout*> binding_layouts;
			std::span<ISampler*> immutable_samplers;
		};

		// Vertex input
		struct VertexAttributeDescription {
			uint32_t location{ 0u };
			uint32_t binding{ 0u };
			Format format{ Format::eUndefined };
			uint32_t offset{ 0u };
		};

		void to_json(nlohmann::json& json, const VertexAttributeDescription& type);
		void from_json(const nlohmann::json& json, VertexAttributeDescription& type);

		struct VertexBindingDescription {
			uint32_t binding{ 0u };
			uint32_t stride{ 0u };
		};

		void to_json(nlohmann::json& json, const VertexBindingDescription& type);
		void from_json(const nlohmann::json& json, VertexBindingDescription& type);

		struct VertexLayout {
			std::array<VertexBindingDescription, 16ull> bindings{};
			uint32_t binding_count{ 0u };
			std::array<VertexAttributeDescription, 16ull> attributes{};
			uint32_t attribute_count{ 0u };
		};

		void to_json(nlohmann::json& json, const VertexLayout& type);
		void from_json(const nlohmann::json& json, VertexLayout& type);

		struct RasterizationState {
			bool depth_clamp{ false };
			bool discard_enable{ false };
			FillMode fill_mode{ FillMode::eSolid };
			CullModeFlags cull_mode{ CullModeFlag::eNone };
			FrontFace front_face{ FrontFace::eCCW };
		};

		void to_json(nlohmann::json& json, const RasterizationState& type);
		void from_json(const nlohmann::json& json, RasterizationState& type);

		struct MultisampleState {
			SampleCount sample_count{ SampleCount::e1 };
			bool alpha_to_coverage{ false };
			bool alpha_to_one{ false };
		};

		void to_json(nlohmann::json& json, const MultisampleState& type);
		void from_json(const nlohmann::json& json, MultisampleState& type);

		struct DepthStencilState {
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

		void to_json(nlohmann::json& json, const DepthStencilState& type);
		void from_json(const nlohmann::json& json, DepthStencilState& type);

		struct ColorBlendAttachment {
			bool enable{ false };
			BlendFactor src_color{ BlendFactor::eOne };
			BlendFactor dst_color{ BlendFactor::eZero };
			BlendOp color_blend_op{ BlendOp::eAdd };
			BlendFactor src_alpha{ BlendFactor::eOne };
			BlendFactor dst_alpha{ BlendFactor::eZero };
			BlendOp alpha_blend_op{ BlendOp::eAdd };
			ColorMaskFlags color_mask{ ColorMaskFlag::eAll };
		};

		void to_json(nlohmann::json& json, const ColorBlendAttachment& type);
		void from_json(const nlohmann::json& json, ColorBlendAttachment& type);

		struct ColorBlendingState {
			std::array<ColorBlendAttachment, k_max_attachment_count> attachments{};
			uint32_t attachment_count{ 0u };
		};

		void to_json(nlohmann::json& json, const ColorBlendingState& type);
		void from_json(const nlohmann::json& json, ColorBlendingState& type);

		struct PipelineState {
			VertexLayout vertex_layout{};
			RasterizationState rasterization_state{};
			MultisampleState multisample_state{};
			DepthStencilState depth_stencil_state{};
			ColorBlendingState color_blending_state{};

			PrimitiveTopology primitive_topology{ PrimitiveTopology::ePointList };
		};

		void to_json(nlohmann::json& json, const PipelineState& type);
		void from_json(const nlohmann::json& json, PipelineState& type);

		struct GraphicsPipelineCreateInfo {
			IShader* shader{ nullptr };
			IRootSignature* root_signature{ nullptr };
			IPipelineCache* pipeline_cache{ nullptr };

			PipelineState pipeline_state{};

			std::array<Format, k_max_attachment_count> color_formats{};
			uint32_t color_format_count{};

			Format depth_format{ Format::eUndefined };
			Format stencil_format{ Format::eUndefined };
		};

		struct ComputePipelineCreateInfo {
			IShader* shader{ nullptr };
			IRootSignature* root_signature{ nullptr };
			IPipelineCache* pipeline_cache{ nullptr };
		};

		struct ClearColor {
			ClearColor() = default;

			ClearColor(float r, float g, float b, float a) :
				color{ r, g, b, a }
			{}

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

		struct Attachment {
			ITexture* texture{ nullptr };
			ResourceStateFlags texture_state{};

			LoadOp load_op{ LoadOp::eLoad };
			StoreOp store_op{ StoreOp::eStore };

			ClearColor clear_color{};
		};

		struct RenderingInfo {
			int32_t offset_x{ 0u };
			int32_t offset_y{ 0u };

			uint32_t width{ 1u };
			uint32_t height{ 1u };

			uint32_t layer_count{ 1u };

			std::array<Attachment, k_max_attachment_count> attachments{};
			uint32_t attachment_count{ 0u };

			Attachment depth_attachment{};
			Attachment stencil_attachment{};
		};

		struct PipelineCacheCreateInfo {
			std::span<uint8_t> data{};
		};

		struct SwapChainCreateInfo {
			const Window* window{ nullptr };
			ISwapchain* old_swapchain{ nullptr };

			uint32_t image_count{ 1u };
			uint32_t width{ 1u };
			uint32_t height{ 1u };

			// TODO: add colorspace selection
			Format format{ Format::eUndefined };
		};

		struct BufferCreateInfo {
			BufferUsageFlags usage{};
			size_t size{ 1ull };
			size_t count{ 1ull };
		};

		void to_json(nlohmann::json& json, const BufferCreateInfo& type);
		void from_json(const nlohmann::json& json, BufferCreateInfo& type);

		struct TextureCreateInfo {
			TextureUsageFlags usage{};

			uint32_t width{ 1u };
			uint32_t height{ 1u };
			uint32_t depth{ 1u };

			uint32_t layers{ 1u };
			uint32_t levels{ 1u };

			Format format{ Format::eUndefined };
		};

		void to_json(nlohmann::json& json, const TextureCreateInfo& type);
		void from_json(const nlohmann::json& json, TextureCreateInfo& type);

		struct SamplerCreateInfo {
			Filter min_filter{ Filter::eNearest };
			Filter mag_filter{ Filter::eNearest };
			Filter mip_map_mode{ Filter::eNearest };
			SamplerAddressMode address_u{ SamplerAddressMode::eRepeat };
			SamplerAddressMode address_v{ SamplerAddressMode::eRepeat };
			SamplerAddressMode address_w{ SamplerAddressMode::eRepeat };
			float mip_lod_bias{ 0.f };
			float min_lod{ 0.f };
			float max_lod{ 12.f };
			float max_anisotropy{ 0.f };
			CompareOp depth_compare{ CompareOp::eNone };
		};

		void to_json(nlohmann::json& json, const SamplerCreateInfo& type);
		void from_json(const nlohmann::json& json, SamplerCreateInfo& type);

		struct QueryPoolCreateInfo {
			QueryType type{};
			uint32_t query_count{ 0u };
		};

		// Resource barriers
		struct BufferRangeInfo {
			uint64_t size{ 0ull };
			uint64_t offset{ 0ull };
		};

		struct BufferBarrierDesc {
			IBuffer* buffer{ nullptr };
			ResourceStateFlags src_state{};
			ResourceStateFlags dst_state{};
			BufferRangeInfo range{};
		};

		struct TextureBarrierDesc {
			ITexture* texture{ nullptr };
			uint32_t subresource_index{ ~0u };

			ResourceStateFlags src_state{};
			ResourceStateFlags dst_state{};
		};

		struct MemoryBarrierDesc {
			ResourceStateFlags src_state{};
			ResourceStateFlags dst_state{};
		};

		struct ResourceBarrierInfo {
			std::span<const BufferBarrierDesc> buffer_barriers;
			std::span<const TextureBarrierDesc> texture_barriers;
			std::span<const MemoryBarrierDesc> memory_barriers;
		};

		struct IndirectDrawCommand {
			bool indexed_draw{ false };

			IBuffer* argument_buffer{ nullptr };
			uint64_t argument_offset{ 0ull };
			uint32_t draw_count{ 0u };
			uint32_t command_stride{ 0u };

			IBuffer* count_buffer{ nullptr };
			uint64_t count_offset{ 0ull };
		};

		struct BufferCopyRegion {
			uint64_t src_offset{ 0ull };
			uint64_t dst_offset{ 0ull };
			uint64_t size{ 0ull };
		};

		struct CopyBufferInfo {
			IBuffer* src_buffer{ nullptr };
			IBuffer* dst_buffer{ nullptr };

			std::span<BufferCopyRegion> regions;
		};

		struct TextureCopyRegion {
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

		struct CopyTextureInfo {
			IBuffer* src_buffer{ nullptr };
			ITexture* dst_texture{ nullptr };

			std::span<TextureCopyRegion> copy_regions{};
		};

		struct BindDescriptorsInfo {
			PipelineType pipeline_type{}; 
			IRootSignature* root_signature{ nullptr };

			uint32_t first_set{ 0u };
			std::span<IBindingLayout*> binding_layouts{};
		};

		struct BufferDescriptor {
			DescriptorType type{};

			IBuffer* buffer{ nullptr };

			uint32_t binding{ 0u };

			uint32_t offset{ 0u };
			uint32_t size{ 0u };
		};

		struct SamplerDescriptor {
			ISampler* sampler{ nullptr };
			uint32_t binding{ 0u };
			uint32_t array_index{ 0u };
		};

		struct TextureDescriptor {
			DescriptorType type{};

			ITexture* texture{ nullptr };
			ISampler* sampler{ nullptr };
			uint32_t subresource_index{ ~0u };
			ResourceStateFlags resource_state{};

			uint32_t binding{ 0u };
			uint32_t array_index{ 0u };
		};

		struct DescrptorLayoutBindingDesc {
			uint32_t binding;
			uint32_t count;
			ShaderStageFlags stages;
			DescriptorType type;
		};

		struct DescriptorLayoutCreateInfo {
			std::span<DescrptorLayoutBindingDesc> bindings;
			bool support_bindless{ false };
		};

		struct RenderResourceDesc {
			IGFXResource* resource{ nullptr };
			ResourceStateFlags state{};
		};

		struct AsyncResourceHandle {
			IGFXResource* resource{ nullptr };
			ResourceStateFlags state{};
			std::atomic_bool done{ false };
		};

		uint32_t hash(const VertexAttributeDescription& desc, uint32_t hash = 0u);
		uint32_t hash(const VertexBindingDescription& desc, uint32_t hash = 0u);
		uint32_t hash(const VertexLayout& layout, uint32_t hash = 0u);
		uint32_t hash(const RasterizationState& state, uint32_t hash = 0u);
		uint32_t hash(const MultisampleState& state, uint32_t hash = 0u);
		uint32_t hash(const DepthStencilState& state, uint32_t hash = 0u);
		uint32_t hash(const ClearColor& color, uint32_t hash = 0u);
		uint32_t hash(const ColorBlendAttachment& attachment, uint32_t hash = 0u);
		uint32_t hash(const ColorBlendingState& state, uint32_t hash = 0u);
		uint32_t hash(const PipelineState& state, uint32_t hash = 0u);

		uint32_t hash(const SamplerCreateInfo& create_info, uint32_t hash = 0u);

		uint32_t hash(const DescrptorLayoutBindingDesc& desc, uint32_t hash = 0u);
	}
}