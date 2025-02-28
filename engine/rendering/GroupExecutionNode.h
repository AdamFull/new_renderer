#pragma once

#include "ExecutionContext.h"

namespace engine
{
	namespace render
	{
		class GroupNode final : public ExecutionNode
		{
		public:
			GroupNode(std::string_view name, uint32_t color = 0xFFFFFFFF);
			~GroupNode() = default;

			void AddNode(std::unique_ptr<ExecutionNode>&& node);

			void SetGroupEnabledCallback(std::function<bool()>&& callback);

			void Execute(edge::gfx::ICommandList& command_list, float delta_time) override;
		private:
			std::string group_name{};
			uint32_t group_color{ 0xFFFFFFFF };

			std::function<bool()> enabled_callback{};
			std::vector<std::unique_ptr<ExecutionNode>> graph_nodes{};
		};
	}
}