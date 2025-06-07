#pragma once

#include "../util/utl_hash.hpp"
#include "gfx_structures.h"

namespace edge::gfx {
	struct FGResource {
		IGFXResource* resource{ nullptr };
		ResourceStateFlags state{ ResourceStateFlag::eUndefined };
		bool external{ false };
	};

	struct FGResourceDesc {
		// Binding point in shader to get more info about resource
		uint32_t binding_point{ ~0u }; 
		uint32_t handle{ ~0u };
	};

	struct FGPass {
		IShader* shader{ nullptr };
		IRootSignature* root_sign{ nullptr };
		IPipeline* pipeline{ nullptr };
		edge::utl::fixed_vector<edge::gfx::IDescriptorLayout*> descriptor_layouts;

		std::vector<FGResourceDesc> read_resources{};
		std::vector<FGResourceDesc> write_resources{};

		// TODO: Need to think can pass create resources by it's own or not
	};

	class IFrameGraph {
	public:
		virtual ~IFrameGraph() = default;

		virtual auto add_resource(std::string_view name, const TextureCreateInfo& create_info) -> uint32_t = 0;
		virtual auto add_resource(std::string_view name, const BufferCreateInfo& create_info) -> uint32_t = 0;
		virtual auto add_resource(std::string_view name, IGFXResource* resource) -> uint32_t = 0;
		virtual void update_resource(uint32_t handle, IGFXResource* resource) = 0;
		// TODO: Add remove resource functions

		virtual auto get_resource(uint32_t handle) -> FGResource& = 0;

		virtual auto add_pass() -> uint32_t = 0;
		virtual auto build() -> bool = 0;
		virtual auto execute(ICommandList& cmd_list, float delta_time) -> bool = 0;
	};

	[[nodiscard]] auto create_frame_graph() -> IFrameGraph*;
	[[nodiscard]] auto create_frame_graph_unique() -> std::unique_ptr<IFrameGraph>;
}