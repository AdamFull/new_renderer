#pragma once

#include <edge/gfx/gfx_base.h>

namespace e2d {
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

		virtual auto evaluate() -> void = 0;
		virtual auto get_resource() -> edge::gfx::IGFXResource* { return resource; }
		virtual auto set_resource(edge::gfx::IGFXResource* new_res) -> void { resource = new_res; }
		virtual auto get_state() -> edge::gfx::ResourceStateFlags { return state; }
		virtual auto set_state(edge::gfx::ResourceStateFlags new_state) -> void { state = new_state; }
		virtual auto get_size() -> size_t = 0;

	protected:
		edge::gfx::IGFXResource* resource{ nullptr };
		RGResourceVersion version{ 0u };

		std::string name{};
		edge::gfx::ResourceStateFlags state{ edge::gfx::ResourceStateFlag::eUndefined };
		bool is_imported{ false };
		bool is_persistent{ false };
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

		virtual auto execute(edge::gfx::ICommandList& cmd_list, float delta_time) -> void = 0;
	};

	using RGPassSetupFn = std::function<void(IRGPass&)>;
	using RGPassExecuteFn = std::function<void(edge::gfx::ICommandList&, float)>;

	struct RGPassCreateInfo {
		std::string name;
		std::string technique_name;

		RGPassSetupFn setup_func;
		RGPassExecuteFn execute_func;
	};

	class IRenderGraph {
	public:
		virtual ~IRenderGraph() = default;

		virtual auto create_buffer(const edge::gfx::BufferCreateInfo& create_info, std::string_view name = "") -> RGResourceHandle = 0;
		virtual auto create_texture(const edge::gfx::TextureCreateInfo& create_info, std::string_view name = "") -> RGResourceHandle = 0;

		virtual auto import_buffer(edge::gfx::IBuffer* buffer, std::string_view name = "") -> RGResourceHandle = 0;
		virtual auto import_texture(edge::gfx::ITexture* texture, std::string_view name = "") -> RGResourceHandle = 0;

		virtual auto update_buffer(RGResourceHandle handle, edge::gfx::IBuffer* buffer) -> RGResourceHandle = 0;
		virtual auto update_texture(RGResourceHandle handle, edge::gfx::ITexture* texture) -> RGResourceHandle = 0;

		virtual auto add_pass(RGPassCreateInfo create_info) -> void = 0;

		virtual auto get_resource(RGResourceHandle handle) -> IRGResource* = 0;

		virtual auto execute(edge::gfx::ICommandList& cmd_list, float delta_time) -> void = 0;
	};
}