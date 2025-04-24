#pragma once

#include "ExecutionContext.h"

namespace engine::render {
	class GroupNode final : public IExecutionNode {
	public:
		using CallbackType = std::function<void(edge::gfx::ICommandList&, float)>;

		GroupNode(std::string_view name, uint32_t color = 0xFFFFFFFF);
		~GroupNode() = default;

		void add_node(std::unique_ptr<IExecutionNode>&& node);
		void set_pre_callback(CallbackType&& callback) {
			pre_callback = std::move(callback);
		}

		void set_post_callback(CallbackType&& callback) {
			post_callback = std::move(callback);
		}

		void execute(edge::gfx::ICommandList& command_list, float delta_time) override;
	private:
		std::string group_name{};
		uint32_t group_color{ 0xFFFFFFFF };

		CallbackType pre_callback{};
		CallbackType post_callback{};
		std::vector<std::unique_ptr<IExecutionNode>> graph_nodes{};
	};
}