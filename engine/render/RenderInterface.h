#pragma once

#include <interface/RenderInterface.h>
#include "ResourceLoader.h"
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

			size_t createTexture(const std::string& name, const std::vector<uint8_t>& data, EMimeType mime) override;

			void updateSurfaceSize(int32_t width, int32_t height) override;

			void beginFrame() override;
			void endFrame() override;
		private:
			std::unique_ptr<IRenderHandleInterface> renderHandle{};
			std::unique_ptr<CResourceLoader> resourceLoader{};
		};
	}
}