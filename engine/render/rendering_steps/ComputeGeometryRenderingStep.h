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
			CComputeGeometryRenderingStep(IRenderInterface* render_interface);

			~CComputeGeometryRenderingStep() override = default;

			void _create(const FRenderingStepCreateInfo& create_info) override;
			void _execute(f32 dt) override;
		protected:
			const std::unique_ptr<IShaderObject>& getShaderByVBO(const IVertexHandler* vbo);
		private:
			std::unordered_map<u64, std::unique_ptr<IShaderObject>> m_shaders{};
			std::string m_baseShaderName{};

			std::unordered_map<std::string, u64> m_shaderInputs{};
		};
	}
}