#pragma once

#include "IRenderGraph.h"

namespace edge::gfx {
	class TechniqueDatabase;
}

namespace e2d {
	class RenderGraph final : public IRenderGraph {
	public:
		friend class GraphPassDescriptor;
		RenderGraph(edge::gfx::TechniqueDatabase& technique_database);
		~RenderGraph() override;

		static auto construct(edge::gfx::TechniqueDatabase& technique_database) -> std::unique_ptr<IRenderGraph>;

		auto create_resource(const RGResourceCreateInfo& create_info) -> RGResourceHandle override;
		auto update_resource(RGResourceHandle handle, edge::gfx::IGFXResource* buffer) -> RGResourceHandle override;

		auto add_pass(RGPassCreateInfo create_info) -> void override;

		auto get_resource(RGResourceHandle handle) -> IRGResource* override;

		auto execute(edge::gfx::ICommandList& cmd_list, float delta_time) -> void override;
	private:
		RGResourceId next_resource_id_{ 0 };
		auto next_resource_id() -> RGResourceId { return next_resource_id_++; }

		std::unordered_map<RGResourceId, std::unique_ptr<IRGResource>> resources_;
		std::vector<std::unique_ptr<IRGPass>> passes_;

		edge::gfx::TechniqueDatabase& technique_db_;
	};
}