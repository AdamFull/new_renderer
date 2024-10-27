#pragma once

#include <engine/rendering/render_graph/ShaderPass.h>

namespace engine
{
	namespace render
	{
		class ComputePass : public ShaderPass
		{
		public:
			ComputePass(gfx::Context& context, const ComputePassCreateInfo& create_info);
			~ComputePass() = default;

			ShaderPassType GetPassType() override;

		protected:
			void _Execute(gfx::CommandList& command_list, float delta_time) override;
		};
	}
}