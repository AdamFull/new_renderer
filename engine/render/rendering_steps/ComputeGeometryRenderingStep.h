#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CComputeGeometryRenderingStep : public IComputeStep
		{
		public:
			CComputeGeometryRenderingStep() = default;
			CComputeGeometryRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context);

			~CComputeGeometryRenderingStep() override = default;

			void _create(const FRenderingStepCreateInfo& create_info) override;
			void _execute(float dt) override;
		protected:
			const std::unique_ptr<IShaderObject>& getShaderByVBO(const IVertexHandler* vbo);
		private:
			std::unordered_map<size_t, std::unique_ptr<IShaderObject>> m_shaders{};
			std::string m_baseShaderName{};

			std::unordered_map<std::string, size_t> m_shaderInputs{};

			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}