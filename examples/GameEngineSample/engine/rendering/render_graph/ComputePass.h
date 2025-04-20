#pragma once

#include "ShaderPass.h"

namespace engine::render {
	class ComputePass : public ShaderPass {
	public:
		ComputePass(IExecutionContext& execution_context, const ComputePassCreateInfo& create_info);
		virtual ~ComputePass() = default;

		ShaderPassType get_pass_type() override;

		void begin(edge::gfx::ICommandList& command_list) override {}
		void end(edge::gfx::ICommandList& command_list) override {}

	protected:
		void _execute(edge::gfx::ICommandList& command_list, float delta_time) override;
	};
}