#include "GroupExecutionNode.h"

#include <edge/util/utl_profiler.h>
#include <edge/gfx/gfx_profiler.h>

namespace engine::render {
	GroupNode::GroupNode(std::string_view name, uint32_t color) :
		group_name{ name },
		group_color{ color } {
	}

	void GroupNode::add_node(std::unique_ptr<IExecutionNode>&& node) {
		graph_nodes.emplace_back(std::move(node));
	}

	void GroupNode::set_group_enabled_callback(std::function<bool()>&& callback) {
		enabled_callback = std::move(callback);
	}

	void GroupNode::execute(edge::gfx::ICommandList& command_list, float delta_time) {
		EDGE_PROFILE_ZONE_SCOPED;

		if (enabled_callback && !enabled_callback()) {
			return;
		}

		EDGE_GFX_PROFILER_BEGIN_ZONE_NC(command_list, group_name, group_color);
		for (auto& node : graph_nodes) {
			node->execute(command_list, delta_time);
		}
		EDGE_GFX_PROFILER_END_ZONE(command_list);
	}
}