#pragma once

#include <interface/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class IRenderRuleInterface
		{
		private:
			virtual void create(const FRenderingCreateInfo& create_info) = 0;
			virtual void render() = 0;
		protected:
			IRenderHandleInterface* m_pRenderContext{ nullptr };
			IRenderInterface* m_pRenderInterface{ nullptr };
			FBeginRenderingInfo m_renderingInfo{};
		};
	}
}