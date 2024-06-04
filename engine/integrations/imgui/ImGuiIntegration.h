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
			CImGuiIntegration(render::IRenderInterface* render_interface);
			~CImGuiIntegration();

			void create() override;
			void destroy() override;

			void begin();
			void end();
			void render() override;
		protected:
			void createFontTarget();
		private:
			u64 m_shader{ INVALID_HANDLE };
			u64 m_vertexHandler{ INVALID_HANDLE };
			u64 m_fontTexture{ INVALID_HANDLE };
			render::IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}