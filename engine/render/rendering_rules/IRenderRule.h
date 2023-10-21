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
		protected:
			IRenderHandleInterface* renderContext{ nullptr };
		};
	}
}