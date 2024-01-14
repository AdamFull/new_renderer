#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CComputeCullingRenderingStep : public IComputeStep
		{
		public:
			CComputeCullingRenderingStep() = default;
			CComputeCullingRenderingStep(IRenderInterface* render_interface);

			~CComputeCullingRenderingStep() override = default;

			void _create(const FRenderingStepCreateInfo& create_info) override;
			void _execute(float dt) override;
		protected:
			const std::unique_ptr<IShaderObject>& getShaderByVBO(const IVertexHandler* vbo);
		private:
			std::unordered_map<size_t, std::unique_ptr<IShaderObject>> m_shaders{};
			std::string m_baseShaderName{};
		};
	}
}