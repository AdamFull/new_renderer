#pragma once

#include <edge/gfx/gfx_base.h>

namespace e2d {
	class IRenderGraph;

	using RGResourceId = uint32_t;
	constexpr RGResourceId INVALID_RESOURCE_ID = 0;

	using RGResourceVersion = uint32_t;

	class RGResourceHandle {
	public:
		RGResourceHandle() = default;
		explicit RGResourceHandle(RGResourceId id) : resource_id_(id) {}

		[[nodiscard]] RGResourceId id() const noexcept { return resource_id_; }
		[[nodiscard]] bool is_valid() const noexcept { return resource_id_ != INVALID_RESOURCE_ID; }

		bool operator==(const RGResourceHandle& other) const noexcept {
			return resource_id_ == other.resource_id_;
		}

		bool operator!=(const RGResourceHandle& other) const noexcept {
			return !(*this == other);
		}

	private:
		RGResourceId resource_id_ = INVALID_RESOURCE_ID;
	};

	class IRGResource {
	public:
		friend class IRenderGraph;
		virtual ~IRGResource() = default;

		virtual auto evaluate(IRenderGraph& graph) -> void = 0;
		virtual auto get_resource() -> edge::gfx::IGFXResource* { return resource_; }
		virtual auto set_resource(edge::gfx::IGFXResource* new_res) -> void { resource_ = new_res; }
		virtual auto get_state() -> edge::gfx::ResourceStateFlags { return state_; }
		virtual auto set_state(edge::gfx::ResourceStateFlags new_state) -> void { state_ = new_state; }
		virtual auto get_size() -> size_t = 0;

		virtual auto set_parent(RGResourceHandle parent) noexcept -> void { parent_handle_ = parent; }
		virtual auto get_parent() const noexcept -> RGResourceHandle { return parent_handle_; }

		virtual auto add_child(RGResourceHandle child) -> void { child_handles_.emplace_back(child); }

	protected:
		edge::gfx::IGFXResource* resource_{ nullptr };
		RGResourceVersion version_{ 0u };

		std::string name_{};
		edge::gfx::ResourceStateFlags state_{ edge::gfx::ResourceStateFlag::eUndefined };
		bool is_imported_{ false };
		bool is_persistent_{ false };

		RGResourceHandle parent_handle_{ INVALID_RESOURCE_ID };
		std::vector<RGResourceHandle> child_handles_{};
	};

	class IRGPass {
	public:
		virtual ~IRGPass() = default;

		virtual auto add_srv(RGResourceHandle handle) -> void = 0;
		virtual auto add_uav(RGResourceHandle handle, uint32_t subresource_index = ~0u) -> void = 0;

		virtual auto add_vertex_buffer(RGResourceHandle handle) -> void = 0;
		virtual auto add_index_buffer(RGResourceHandle handle) -> void = 0;
		virtual auto add_indirect_buffer(RGResourceHandle handle) -> void = 0;

		virtual auto add_color_attachment(RGResourceHandle handle, edge::gfx::LoadOp load_op = edge::gfx::LoadOp::eDontCare, edge::gfx::ClearColor clear_color = {1.0f, 1.0f, 1.0f, 1.0f}) -> void = 0;
		virtual auto add_depth_attachment(RGResourceHandle handle, edge::gfx::LoadOp load_op = edge::gfx::LoadOp::eDontCare, edge::gfx::ClearColor clear_color = { 1.0f }) -> void = 0;
		virtual auto add_stencil_attachment(RGResourceHandle handle, edge::gfx::LoadOp load_op = edge::gfx::LoadOp::eDontCare, edge::gfx::ClearColor clear_color = { 0u }) -> void = 0;

		virtual auto set_offset(int32_t x, int32_t y) -> void = 0;
		virtual auto set_size(uint32_t width, int32_t height) -> void = 0;
		virtual auto set_layers(uint32_t layer_count) -> void = 0;

		virtual auto get_graph() -> IRenderGraph& = 0;

		virtual auto push_constants(edge::gfx::ICommandList& cmd_list, std::span<const char> data) -> void = 0;

		virtual auto execute(edge::gfx::ICommandList& cmd_list, float delta_time) -> void = 0;
	};

	using RGPassSetupFn = std::function<void(IRGPass&)>;
	using RGPassExecuteFn = std::function<void(IRGPass& , edge::gfx::ICommandList&, float)>;

	struct RGPassCreateInfo {
		std::string name;
		std::string technique_name;

		RGPassSetupFn setup_func;
		RGPassExecuteFn execute_func;
	};

	struct RGResourceCreateInfo {
		std::string name;
		RGResourceHandle parent;
		edge::gfx::ResourceType type;
		edge::gfx::IGFXResource* external_resource{ nullptr };

		union {
			edge::gfx::BufferCreateInfo buffer_create_info;
			struct {
				edge::gfx::TextureCreateInfo texture_create_info;
				std::array<float, 3> scale;
			};
		};
	};

	class IRenderGraph {
	public:
		virtual ~IRenderGraph() = default;

		virtual auto create_resource(const RGResourceCreateInfo& create_info) -> RGResourceHandle = 0;
		virtual auto update_resource(RGResourceHandle handle, edge::gfx::IGFXResource* buffer) -> RGResourceHandle = 0;
		virtual auto add_pass(RGPassCreateInfo create_info) -> void = 0;
		virtual auto get_resource(RGResourceHandle handle) -> IRGResource* = 0;
		virtual auto execute(edge::gfx::ICommandList& cmd_list, float delta_time) -> void = 0;
	};
}