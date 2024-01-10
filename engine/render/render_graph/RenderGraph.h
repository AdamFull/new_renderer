#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CRenderGraph : public IRenderGraph
		{
		public:
			size_t addRenderResource(const std::string& name) override;

			void bake() override;

			void execute(float dt) override;
		private:
			// std::vector<PassInfo>
			std::vector<std::unique_ptr<IRenderPass>> m_vRenderPasses{};
		};
	}
}