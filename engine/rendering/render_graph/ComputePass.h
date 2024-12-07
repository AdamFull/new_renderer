#pragma once

#include "ShaderPass.h"

namespace engine
{
	namespace render
	{
		class ComputePass : public ShaderPass
		{
		public:
			ComputePass(IRenderGraph& graph, const ComputePassCreateInfo& create_info);
			virtual ~ComputePass() = default;

			ShaderPassType GetPassType() override;

			void Begin(gfx::CommandList& command_list) override {}
			void End(gfx::CommandList& command_list) override {}

		protected:
			void _Execute(gfx::CommandList& command_list, float delta_time) override;
		};
	}
}