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
			CComputeRenderingStep(IRenderInterface* render_interface);

			~CComputeRenderingStep() override = default;

			void _create(const FRenderingStepCreateInfo& create_info) override;
			void _execute(f32 dt) override;
		private:
			std::unique_ptr<IShaderObject> m_pShader{};

			std::unordered_map<std::string, u64> m_shaderInputs{};
		};
	}
}