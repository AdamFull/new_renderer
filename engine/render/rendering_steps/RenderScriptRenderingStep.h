#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CRenderScriptRenderingStep : public IRenderingStep
		{
		public:
			CRenderScriptRenderingStep() = default;
			CRenderScriptRenderingStep(IRenderInterface* render_interface);

			~CRenderScriptRenderingStep() override = default;

			void _create(const FRenderingStepCreateInfo& create_info) override;
			void _execute(f32 dt) override;
		};
	}
}