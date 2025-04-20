#pragma once

#include <edge/gfx/gfx_base.h>

namespace engine::render {
	class ShaderPass;

	enum class RenderResourceTypeFlag : uint16_t {
		eRenderTarget = 1 << 0,
		eStorageTarget = 1 << 1,
		eStaticImage = 1 << 2,
		eBackbuffer = 1 << 3,
		eAllImage = eRenderTarget | eStorageTarget | eStaticImage | eBackbuffer,
		eStorageBuffer = 1 << 4,
		eIndirectBuffer = 1 << 5,
		eAllBuffer = eStorageBuffer | eIndirectBuffer
	};
	EDGE_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(RenderResourceTypeFlags, RenderResourceTypeFlag);

	struct RenderResource {
		RenderResourceTypeFlag resource_type{};
		// Id of real resource
		uint32_t resource_id{ ~0u };

		edge::gfx::ResourceStateFlags current_state{};

		bool finalized{ false };

		//std::unordered_set<ResourceAccess> usages{};

		edge::gfx::AccessTypeFlags GetAccess() const {
			if (edge::eq_(current_state, edge::gfx::ResourceStateFlag::eRead)) {
				return edge::gfx::AccessTypeFlag::eRead;
			}
			else if (edge::eq_(current_state, edge::gfx::ResourceStateFlag::eWrite)) {
				return edge::gfx::AccessTypeFlag::eWrite;
			}
			else if (edge::eq_(current_state, edge::gfx::ResourceStateFlag::eRead | edge::gfx::ResourceStateFlag::eWrite)) {
				return edge::gfx::AccessTypeFlag::eRead | edge::gfx::AccessTypeFlag::eWrite;
			}

			return {};
		}

		auto GetState() const noexcept -> edge::gfx::ResourceStateFlags {
			return current_state;
		}

		auto SetState(edge::gfx::ResourceStateFlags new_state) noexcept -> void {
			current_state = new_state;
		}
	};

	enum class ResourceAttachmentTypeFlag {
		eColor = 1 << 0,
		eDepth = 1 << 1,
		eStencil = 1 << 2,
		eDepthStencil = eDepth | eStencil
	};
	EDGE_REGISTER_ENUM_CLASS_AS_FLAG_TYPE(ResourceAttachmentTypeFlags, ResourceAttachmentTypeFlag);

	// Describes out attachment like color or depth/stencil attachment
	struct RenderResourceAttachment {
		edge::gfx::AccessTypeFlags access{};

		ResourceAttachmentTypeFlags flags{};

		// Index of render resource
		uint32_t render_resource_id{ ~0u };

		// Render resource clear color
		edge::gfx::ClearColor clear_color{};
	};

	struct ShaderPassCreateInfo {
		// Shader name in shader cache
		std::string technique_name{};

		std::function<void(ShaderPass&, edge::gfx::ICommandList&, float)> draw_callback{};
		std::function<bool()> enabled_callback{};

		// Means that this pass will be executed only one time
		bool one_time{ false };
	};

	struct ComputeShaderPassCreateInfo : ShaderPassCreateInfo {
	};

	struct RenderShaderPassCreateInfo : ShaderPassCreateInfo
	{
		std::vector<RenderResourceAttachment> resource_attachments{};

		void add_write_attachment(uint32_t render_resource_id, ResourceAttachmentTypeFlags flags = {}, edge::gfx::ClearColor clear_color = {}) { resource_attachments.emplace_back(edge::gfx::AccessTypeFlag::eWrite, flags, render_resource_id, clear_color); }
		void add_read_attachment(uint32_t render_resource_id, ResourceAttachmentTypeFlags flags = {}) { resource_attachments.emplace_back(edge::gfx::AccessTypeFlag::eRead, flags, render_resource_id, edge::gfx::ClearColor{}); }
	};

	struct TextureResourceCreateInfo {
		edge::gfx::TextureUsageFlags usage{};

		uint32_t width{ 1u };
		uint32_t height{ 1u };
		uint32_t depth{ 1u };

		uint32_t layers{ 1u };
		uint32_t levels{ 1u };

		bool resizable{ false };
		float scale{ 1.f };
		uint32_t scale_parent_resource{ ~0u };

		edge::gfx::Format format{ edge::gfx::Format::eUndefined };
	};

	struct TextureObject {
		edge::gfx::ITexture* texture{ nullptr };
		edge::gfx::ISampler* sampler{ nullptr };

		edge::gfx::TextureUsageFlags usage{};
		bool resizable{ false };
		float scale{ 1.f };
		uint32_t scale_parent_resource{ ~0u };
	};

	struct BufferObject {
		edge::gfx::IBuffer* buffer{ nullptr };
	};

	class IExecutionNode {
	public:
		virtual ~IExecutionNode() = default;

		virtual void execute(edge::gfx::ICommandList& command_list, float delta_time) = 0;
	};

	class IExecutionContext {
	public:
		virtual ~IExecutionContext() = default;

		virtual auto add_resource() -> uint32_t = 0;
		virtual auto add_resource_sync(std::string_view path, edge::gfx::ISampler* sampler = nullptr) -> uint32_t = 0;
		virtual auto add_resource(std::string_view path, edge::gfx::ISampler* sampler = nullptr) -> uint32_t = 0;
		virtual auto add_resource(const TextureResourceCreateInfo& create_info, std::span<uint8_t> texture_data) -> uint32_t = 0;
		virtual auto add_resource(const TextureResourceCreateInfo& create_info, edge::gfx::ISampler* sampler = nullptr) -> uint32_t = 0;
		virtual auto add_resource(const edge::gfx::BufferCreateInfo& create_info) -> uint32_t = 0;

		virtual void set_resource_name(uint32_t resource_index, std::string_view name) = 0;

		virtual auto get_resource(uint32_t resource_index) -> RenderResource & = 0;
		virtual auto get_resource(uint32_t resource_index) const ->RenderResource const& = 0;

		virtual auto create_node(std::string_view group_name, uint32_t group_color = 0xFFFFFFFF) -> std::unique_ptr<IExecutionNode> = 0;
		virtual auto create_node(const RenderShaderPassCreateInfo& create_info) -> std::unique_ptr<IExecutionNode> = 0;
		virtual auto create_node(const ComputeShaderPassCreateInfo& create_info) -> std::unique_ptr<IExecutionNode> = 0;

		virtual void add_node(std::unique_ptr<IExecutionNode>&& node) = 0;
		virtual void execute(float delta_time) = 0;

		virtual void update_global_binding(uint32_t binding, std::span<const uint8_t> data) = 0;

		virtual void begin() = 0;
		virtual void end() = 0;

		virtual auto get_texture(uint32_t resource_index) -> edge::gfx::ITexture* = 0;
		virtual auto get_texture(const RenderResource& render_resource) -> edge::gfx::ITexture* = 0;

		virtual auto get_sampler(uint32_t resource_index) -> edge::gfx::ISampler* = 0;
		virtual auto get_sampler(const RenderResource& render_resource) -> edge::gfx::ISampler* = 0;

		virtual auto get_buffer(uint32_t resource_index) -> edge::gfx::IBuffer* = 0;
		virtual auto get_buffer(const RenderResource& render_resource) -> edge::gfx::IBuffer* = 0;

		virtual auto request_uniform_block(uint64_t block_size) -> edge::gfx::BufferSuballocation = 0;
		virtual auto get_current_command_list() -> edge::gfx::ICommandList & = 0;

		virtual auto get_binding_layouts(std::span<edge::gfx::IDescriptorLayout*> binding_layouts) -> std::vector<edge::gfx::IBindingLayout*> = 0;
	};
}