#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CCascadedShadowMapsRenderingStep : public IRenderingStep
		{
		public:
			CCascadedShadowMapsRenderingStep() = default;
			CCascadedShadowMapsRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context);

			~CCascadedShadowMapsRenderingStep() override = default;

			void _create(const FRenderingStepCreateInfo& create_info) override;
			void _execute(float dt) override;
		protected:
			const std::unique_ptr<IShaderObject>& getShaderByVBO(const IVertexHandler* vbo);

		private:
			std::unordered_map<size_t, std::unique_ptr<IShaderObject>> m_shaders{};
			std::string m_baseShaderName{};

			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}