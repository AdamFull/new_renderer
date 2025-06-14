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

		auto create_buffer(const edge::gfx::BufferCreateInfo& create_info, std::string_view name = "") -> RGResourceHandle override;
		auto create_texture(const edge::gfx::TextureCreateInfo& create_info, std::string_view name = "") -> RGResourceHandle override;

		auto import_buffer(edge::gfx::IBuffer* buffer, std::string_view name = "") -> RGResourceHandle override;
		auto import_texture(edge::gfx::ITexture* texture, std::string_view name = "") -> RGResourceHandle override;

		auto update_buffer(RGResourceHandle handle, edge::gfx::IBuffer* buffer) -> RGResourceHandle override;
		auto update_texture(RGResourceHandle handle, edge::gfx::ITexture* texture) -> RGResourceHandle override;

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