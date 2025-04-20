#pragma once

#include "ExecutionContext.h"

namespace engine::render {
	class GroupNode final : public IExecutionNode {
	public:
		GroupNode(std::string_view name, uint32_t color = 0xFFFFFFFF);
		~GroupNode() = default;

		void add_node(std::unique_ptr<IExecutionNode>&& node);
		void set_group_enabled_callback(std::function<bool()>&& callback);

		void execute(edge::gfx::ICommandList& command_list, float delta_time) override;
	private:
		std::string group_name{};
		uint32_t group_color{ 0xFFFFFFFF };

		std::function<bool()> enabled_callback{};
		std::vector<std::unique_ptr<IExecutionNode>> graph_nodes{};
	};
}