#pragma once

#include "../util/utl_hash.hpp"
#include "gfx_structures.h"

namespace edge::gfx {
	class IFrameGraph;

	// Flags to describe what i need to inherit from parent resource
	enum class FGResourceDependencyFlag {
		eUsage = 1 << 0,
		eDimensions = 1 << 1,
		eLayers = 1 << 2,
		eLevels = 1 << 3,
		eFormat = 1 << 4,
		eAll = eUsage | eDimensions | eLayers | eLevels | eFormat
	};

	enum class FGPassType {
		eCompute,
		eGraphics,
		eRayTrace
	};

	using FGResourceHandle = uint32_t;
	using FGPassHandle = uint32_t;

	struct FGResourceCreateInfo {
		std::string_view name;
		// The base resource from which the parameters will be inherited
		FGResourceHandle parent_handle{ ~0u };
		FGResourceDependencyFlags dependency_flags{};
		// External resource created outside of graph
		IGFXResource* external{ nullptr };
	};

	struct FGTextureResourceCreateInfo final : FGResourceCreateInfo {
		TextureCreateInfo create_info{};
	};

	struct FGBufferResourceCreateInfo final : FGResourceCreateInfo {
		BufferCreateInfo create_info{};
	};

	struct FGPassCreateInfo {
		std::string_view name;
		FGPassType type{ FGPassType::eGraphics };
		QueueType queue_type{ QueueType::eDirect };
		std::function<void(ICommandList&, float)> execute_func;
		std::function<void()> setup_func;
		std::function<void()> teardown_func;
		std::vector<uint32_t> input_resources;
		std::vector<uint32_t> output_resources;
	};

	class IFGResource {
	public:
		virtual ~IFGResource() = default;

		virtual auto evaluate(IFrameGraph& graph) -> bool = 0;
		virtual auto create_resources(const FGResourceCreateInfo& texture_ci) -> bool;
		virtual auto get_resource() -> IGFXResource* { return resource; }
		virtual auto get_state() const -> ResourceStateFlags { return state; }
		virtual auto set_state(ResourceStateFlags new_state) -> void { state = new_state; }
		virtual auto is_imported() const -> bool { return imported; }
		virtual auto has_parent() const -> bool { return parent != !0u; }
		virtual auto has_children() const -> bool { return !children.empty(); }

		virtual auto set_parent(FGResourceHandle handle) -> void { parent = handle; }
		virtual auto add_children(FGResourceHandle handle) -> void { children.push_back(handle); }
	protected:
		std::string name{};
		// Resource handle IBuffer/ITexture
		IGFXResource* resource{ nullptr };
		// Last resource usage state
		ResourceStateFlags state{ ResourceStateFlag::eUndefined };
		// Externally allocated, not graph resource
		bool imported{ false };

		FGResourceDependencyFlags dependency_flags{};

		FGResourceHandle parent{ ~0u };
		std::vector<FGResourceHandle> children{};
	};

	class IFGPass {
	public:
		virtual ~IFGPass() = default;

		virtual auto execute(ICommandList& cmd_list, float delta_time) -> void = 0;
		virtual auto get_type() const -> FGPassType = 0;
		virtual auto get_queue_type() const -> QueueType = 0;
		virtual auto setup() -> void = 0;
		virtual auto teardown() -> void = 0;
	protected:
		IShader* shader{ nullptr };
		IRootSignature* root_sign{ nullptr };
		IPipeline* pipeline{ nullptr };
		// Resource binding layouts
		edge::utl::fixed_vector<edge::gfx::IDescriptorLayout*> descriptor_layouts;
	};

	class IFrameGraph {
	public:
		virtual ~IFrameGraph() = default;

		virtual auto add_resource(const FGTextureResourceCreateInfo& create_info) -> FGResourceHandle = 0;
		virtual auto add_resource(const FGBufferResourceCreateInfo& create_info) -> FGResourceHandle = 0;
		virtual auto add_pass(const FGPassCreateInfo& create_info) -> FGResourceHandle = 0;
		virtual void update_resource(FGResourceHandle handle, IGFXResource* resource) = 0;
		virtual void remove_resource(FGResourceHandle handle) = 0;
		virtual void remove_pass(FGPassHandle handle) = 0;

		virtual auto get_resource(FGResourceHandle handle) -> IFGResource& = 0;
		virtual auto get_pass(FGPassHandle handle) -> IFGPass& = 0;

		// TODO: need render pass
		virtual auto build() -> bool = 0;
		virtual auto execute(ICommandList& cmd_list, float delta_time) -> bool = 0;
	};

	[[nodiscard]] auto create_frame_graph() -> IFrameGraph*;
	[[nodiscard]] auto create_frame_graph_unique() -> std::unique_ptr<IFrameGraph>;
}

EDGE_REGISTER_ENUM_FLAGS(edge::gfx, FGResourceDependencyFlags, FGResourceDependencyFlag,
	EDGE_ENUM_META(edge::gfx::FGResourceDependencyFlag::eUsage, "usage"),
	EDGE_ENUM_META(edge::gfx::FGResourceDependencyFlag::eDimensions, "dimensions"),
	EDGE_ENUM_META(edge::gfx::FGResourceDependencyFlag::eLayers, "layers"),
	EDGE_ENUM_META(edge::gfx::FGResourceDependencyFlag::eLevels, "levels"),
	EDGE_ENUM_META(edge::gfx::FGResourceDependencyFlag::eFormat, "format")
)