#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CScopedDebugMarker
		{
		public:
			CScopedDebugMarker(IRenderGraph* render_graph, const std::string& name, u32 color, EQueueTypeFlagBits queue = EQueueTypeFlagBits::eGraphics);
			~CScopedDebugMarker();
		private:
			IRenderGraph* m_pRenderGraph{ nullptr };
			std::string m_srName{};
			EQueueTypeFlagBits m_queue{};
		};
	}
}

#define RENDER_DEBUG_MARKER_ZONE(name, ...) CScopedDebugMarker name(this, #name, __VA_ARGS__)