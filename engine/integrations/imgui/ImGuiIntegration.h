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
			u64 m_shader{ INVALID_HANDLE };
			u64 m_vertexHandler{ INVALID_HANDLE };
			u64 m_fontTexture{ INVALID_HANDLE };
			render::IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}