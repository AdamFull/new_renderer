#pragma once

#include "ShaderPass.h"

namespace engine
{
	namespace render
	{
		class ComputePass : public ShaderPass
		{
		public:
			ComputePass(ExecutionContext& execution_context, const ComputePassCreateInfo& create_info);
			virtual ~ComputePass() = default;

			ShaderPassType GetPassType() override;

			void Begin(edge::gfx::CommandList& command_list) override {}
			void End(edge::gfx::CommandList& command_list) override {}

		protected:
			void _Execute(edge::gfx::CommandList& command_list, float delta_time) override;
		};
	}
}