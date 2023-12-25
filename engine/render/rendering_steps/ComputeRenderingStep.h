#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CComputeRenderingStep : public IComputeStep
		{
		public:
			CComputeRenderingStep() = default;
			CComputeRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context);

			~CComputeRenderingStep() override = default;

			void _create(const FRenderingStepCreateInfo& create_info) override;
			void _execute(float dt) override;
		private:
			std::unique_ptr<IShaderObject> m_pShader{};

			std::unordered_map<std::string, size_t> m_shaderInputs{};

			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}