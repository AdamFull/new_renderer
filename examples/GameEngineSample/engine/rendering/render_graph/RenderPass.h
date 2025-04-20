#pragma once

#include "ShaderPass.h"

namespace engine::render {
	class RenderPass : public ShaderPass {
	public:
		RenderPass(IExecutionContext& execution_context, const RenderPassCreateInfo& create_info);
		virtual ~RenderPass() = default;

		void add_color_attachment(edge::gfx::Attachment attachment);

		ShaderPassType get_pass_type() override;

		void begin(edge::gfx::ICommandList& command_list) override;
		void end(edge::gfx::ICommandList& command_list) override;

	protected:
		void _execute(edge::gfx::ICommandList& command_list, float delta_time) override;

	private:
		edge::gfx::RenderingInfo rendering_info{};
		std::vector<RenderResourceAttachment> resource_attachments{};
	};
}