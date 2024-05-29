#pragma once

#include <engine/interface/render/RenderInterface.h>
//#include <engine/render/render_api/vulkan/VkResourceStructures.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class CVkImGuiIntegration
			{
			public:
				~CVkImGuiIntegration() = default;

				void init();
				void destroy();

				void preRender();
				void postRender();
			private:
				class CVkRenderContext* m_pRenderContext{ nullptr };
				//FVkDescriptorPool m_descriptorPool{};
			};
		}
	}
}