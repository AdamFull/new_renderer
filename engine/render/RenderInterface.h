#pragma once

#include <interface/RenderInterface.h>
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

			void updateSurfaceSize(int32_t width, int32_t height) override;

			void beginFrame() override;
			void endFrame() override;
		protected:
			void processMessage(const FRenderMessage& message);
		private:
			std::unique_ptr<IRenderHandleInterface> render_handle{};
		};
	}
}