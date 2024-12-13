#pragma once

#include "ShaderPass.h"

namespace engine
{
	namespace render
	{
		class RenderPass : public ShaderPass
		{
		public:
			RenderPass(ExecutionContext& execution_context, const RenderPassCreateInfo& create_info);
			virtual ~RenderPass() = default;

			void AddColorAttachment(edge::gfx::Attachment attachment);

			ShaderPassType GetPassType() override;

			void Begin(edge::gfx::CommandList& command_list) override;
			void End(edge::gfx::CommandList& command_list) override;

		protected:
			void _Execute(edge::gfx::CommandList& command_list, float delta_time) override;

		private:
			edge::gfx::RenderingInfo rendering_info{};
			std::vector<RenderResourceAttachment> resource_attachments{};
		};
	}
}