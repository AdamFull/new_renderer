#pragma once

#include "IRenderRule.h"

namespace engine
{
	namespace render
	{
		class CScreenspaceRule : public IRenderRuleInterface
		{
		public:
			CScreenspaceRule(IRenderHandleInterface* render_context);

			void create(const FRenderingCreateInfo& create_info) override;
		};
	}
}