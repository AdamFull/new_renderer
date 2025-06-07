#include "gfx_frame_graph.h"

#include "gfx_context.h"

#include "../fs/fs_helper.h"
#include "../util/Logger.h"

#include "../fs/fs_virtual_filesystem.h"

namespace edge::gfx {
	class FrameGraph final : public IFrameGraph {
	public:
		~FrameGraph() override {
			for (auto& resource : resources) {
				if (resource.external) {
					continue;
				}
				destroy(resource.resource);
			}
		}

		auto add_resource(std::string_view name, const TextureCreateInfo& create_info) -> uint32_t override {
			auto handle_id = static_cast<uint32_t>(resources.size());
			resources.emplace_back(create_texture(create_info), ResourceStateFlag::eUndefined, false);
			return handle_id;
		}

		auto add_resource(std::string_view name, const BufferCreateInfo& create_info) -> uint32_t override {
			auto handle_id = static_cast<uint32_t>(resources.size());
			resources.emplace_back(create_buffer(create_info), ResourceStateFlag::eUndefined, false);
			return handle_id;
		}

		auto add_resource(std::string_view name, IGFXResource* resource) -> uint32_t override {
			auto handle_id = static_cast<uint32_t>(resources.size());
			resources.emplace_back(resource, ResourceStateFlag::eUndefined, true);
			return handle_id;
		}

		void update_resource(uint32_t handle, IGFXResource* resource) override {
			// TODO: Check is resource external
			auto& fgresource = resources[handle];
			fgresource.resource = resource;
			fgresource.state = ResourceStateFlag::eUndefined;
		}

		auto get_resource(uint32_t handle) -> FGResource & override {
			return resources[handle];
		}

		auto add_pass() -> uint32_t override {
			// TODO: Need to implement
			return ~0u;
		}

		auto build() -> bool override {
			return false;
		}

		auto execute(ICommandList& cmd_list, float delta_time) -> bool override {

			// Clean graph passes after execution
			passes.clear();

			return false;
		}
	private:
		std::vector<FGPass> passes{};
		std::vector<FGResource> resources{};
	};

	auto create_frame_graph() -> IFrameGraph* {
		return new FrameGraph();
	}

	auto create_frame_graph_unique() -> std::unique_ptr<IFrameGraph> {
		return std::make_unique<FrameGraph>();
	}

}