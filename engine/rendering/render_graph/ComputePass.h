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
			virtual ~ComputePass() = default;

			ShaderPassType GetPassType() override;

		protected:
			void _Execute(gfx::CommandList& command_list, float delta_time) override;
		private:
			bool divide_by_locals{ false };

			gfx::Texture* texture_size_target{ nullptr };
			uint32_t texture_subresource_index{ ~0u };

			gfx::Buffer* buffer_size_target{ nullptr };
		};
	}
}