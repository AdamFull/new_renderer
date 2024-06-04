#pragma once

#include <engine/interface/IntegrationInterface.h>
#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace integration
	{
		class CImGuiIntegration : public IIntegration
		{
		public:
			~CImGuiIntegration() = default;

			void create();
			void destroy();

			void render();
		private:
			render::IRenderInterface* m_pRender_inteface{ nullptr };
		};
	}
}