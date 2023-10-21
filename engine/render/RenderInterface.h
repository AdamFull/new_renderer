#pragma once

#include <interface/RenderInterface.h>
#include "ResourceLoader.h"
#include "RenderConfig.h"
#include <memory>

namespace engine
{
	namespace render
	{
		class CRenderInterface : public IRenderInterface
		{
		public:
			~CRenderInterface() override;

			void create(const FRenderInterfaceCreateInfo& render_interface_ci) override;

			void createRendererBehavior(const std::vector<uint8_t>& script) override;

			FResourceView createTexture(const std::string& name, const std::vector<uint8_t>& data, EMimeType mime) override;

			void updateSurfaceSize(int32_t width, int32_t height) override;

			void beginFrame() override;
			void endFrame() override;
		protected:
			FResourceView getShaderNameTag(const std::string& name) override;
			void createRendering(const FRenderingCreateInfo& rendering_ci) override;

		private:
			std::unique_ptr<IRenderHandleInterface> renderHandle{};
			std::unique_ptr<CResourceLoader> resourceLoader{};
			std::unique_ptr<CRenderConfig> renderConfig{};
		};
	}
}