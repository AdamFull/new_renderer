#include "GroupExecutionNode.h"

#include <edge/util/utl_profiler.h>

namespace engine
{
	namespace render
	{
		GroupNode::GroupNode(std::string_view name, uint32_t color) :
			group_name{ name },
			group_color{ color }
		{
		}

		void GroupNode::AddNode(std::unique_ptr<ExecutionNode>&& node)
		{
			graph_nodes.emplace_back(std::move(node));
		}

		void GroupNode::SetGroupEnabledCallback(std::function<bool()>&& callback)
		{
			enabled_callback = std::move(callback);
		}

		void GroupNode::Execute(edge::gfx::ICommandList& command_list, float delta_time)
		{
			EDGE_PROFILE_ZONE_SCOPED;

			if (enabled_callback && !enabled_callback())
				return;

			command_list.begin_marker(group_name.c_str(), group_color);

			for (auto& node : graph_nodes)
				node->Execute(command_list, delta_time);

			command_list.end_marker();
		}
	}
}