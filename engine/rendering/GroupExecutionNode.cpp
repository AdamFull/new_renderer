#include "GroupExecutionNode.h"

#include <tracy/Tracy.hpp>

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

		void GroupNode::Execute(edge::gfx::CommandList& command_list, float delta_time)
		{
			ZoneScoped;

			if (enabled_callback && !enabled_callback())
				return;

			command_list.BeginMarker(group_name.c_str(), group_color);

			for (auto& node : graph_nodes)
				node->Execute(command_list, delta_time);

			command_list.EndMarker();
		}
	}
}