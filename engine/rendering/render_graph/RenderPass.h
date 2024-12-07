#pragma once

#include "ShaderPass.h"

namespace engine
{
	namespace render
	{
		class RenderPass : public ShaderPass
		{
		public:
			RenderPass(IRenderGraph& graph, const RenderPassCreateInfo& create_info);
			virtual ~RenderPass() = default;

			void AddColorAttachment(gfx::Attachment attachment);

			ShaderPassType GetPassType() override;

			void Begin(gfx::CommandList& command_list) override;
			void End(gfx::CommandList& command_list) override;

		protected:
			void _Execute(gfx::CommandList& command_list, float delta_time) override;

		private:
			gfx::RenderingInfo rendering_info{};
			std::vector<RenderResourceAttachment> resource_attachments{};
		};
	}
}