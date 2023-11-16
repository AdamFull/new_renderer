#pragma once

#include <interface/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CGeometryRenderingStep : public IRenderingStep
		{
		public:
			CGeometryRenderingStep() = default;
			CGeometryRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context);

			~CGeometryRenderingStep() override = default;

			void create(const FRenderingStepCreateInfo& create_info) override;
			void execute(float dt) override;

		private:
			FBeginRenderingInfo m_beginRenderingInfo{};
			std::unique_ptr<IShaderObject> m_pShader{};

			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}