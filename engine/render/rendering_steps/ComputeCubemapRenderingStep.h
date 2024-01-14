#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CComputeCubemapRenderingStep : public IComputeStep
		{
		public:
			CComputeCubemapRenderingStep() = default;
			CComputeCubemapRenderingStep(IRenderInterface* render_interface);

			~CComputeCubemapRenderingStep() override = default;

			void _create(const FRenderingStepCreateInfo& create_info) override;
			void _execute(float dt) override;
		private:
			std::unique_ptr<IShaderObject> m_pShader{};

			std::unordered_map<std::string, size_t> m_shaderInputs{};
		};
	}
}