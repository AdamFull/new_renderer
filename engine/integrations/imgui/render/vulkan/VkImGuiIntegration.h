#pragma once

#include <engine/interface/render/RenderInterface.h>
#include <engine/interface/IntegrationInterface.h>
#include <vulkan/vulkan.hpp>

namespace engine
{
	namespace integration
	{
		class CVkImGuiIntegration : public IRenderIntegration
		{
		public:
			~CVkImGuiIntegration() = default;

			void init() override;
			void destroy() override;

			void preRender() override;
			void postRender() override;
		private:
			render::IRenderInterface* m_pRenderInterface{ nullptr };
			vk::DescriptorPool m_descriptorPool{};
			vk::Device m_device{};
			const vk::AllocationCallbacks* m_allocationCallbacks{ nullptr };
		};
	}
}