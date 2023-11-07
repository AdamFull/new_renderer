#pragma once

#include "IRenderRule.h"
#include <ShaderObject.h>

namespace engine
{
	namespace render
	{
		class CScreenspaceRule : public IRenderRuleInterface
		{
		public:
			CScreenspaceRule(IRenderInterface* render_interface, IRenderHandleInterface* render_context);

			void create(const FRenderingCreateInfo& create_info) override;
			void render(float dt) override;
		private:
			std::unique_ptr<CShaderObject> m_pShader{};
		};
	}
}