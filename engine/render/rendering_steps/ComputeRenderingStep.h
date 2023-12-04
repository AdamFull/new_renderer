#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CComputeRenderingStep : public IRenderingStep
		{
		public:
			CComputeRenderingStep() = default;
			CComputeRenderingStep(IRenderInterface* render_interface, IRenderContext* render_context);

			~CComputeRenderingStep() override = default;

			void create(const FRenderingStepCreateInfo& create_info) override;
			void execute(float dt) override;
		protected:
			const std::unique_ptr<IShaderObject>& getShaderByVBO(const IVertexHandler* vbo);
		private:
			FBeginRenderingInfo m_beginRenderingInfo{};
			std::unordered_map<size_t, std::unique_ptr<IShaderObject>> m_shaders{};
			std::string m_baseShaderName{};

			std::unordered_map<std::string, size_t> m_shaderInputs{};

			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}