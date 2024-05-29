#pragma once

#include <vulkan/vulkan.hpp>
#include <engine/interface/render/RenderInterface.h>
#include <engine/render/render_api/vulkan/VkResourceStructures.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class CImGuiIntegration : public IVKIntegration
			{
			public:
				~CImGuiIntegration() = default;

				void init() override;
				void destroy() override;

				void preRender() override;
				void render(f32 dt) override;
			private:
				class CVkRenderContext* m_pRenderContext{ nullptr };
				FVkDescriptorPool m_descriptorPool{};
			};
		}
	}
}