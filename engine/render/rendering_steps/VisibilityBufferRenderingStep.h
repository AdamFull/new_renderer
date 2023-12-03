#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CVisibilityBufferRenderingStep : public IRenderingStep
		{
		public:
			CVisibilityBufferRenderingStep() = default;
			CVisibilityBufferRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context);

			~CVisibilityBufferRenderingStep() override = default;

			void create(const FRenderingStepCreateInfo& create_info) override;
			void execute(float dt) override;
		protected:
			const std::unique_ptr<IShaderObject>& getShaderByVBO(const std::unique_ptr<IVertexHandler>& vbo);

		private:
			FBeginRenderingInfo m_beginRenderingInfo{};
			std::unordered_map<size_t, std::unique_ptr<IShaderObject>> m_shaders{};
			std::string m_baseShaderName{};

			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}