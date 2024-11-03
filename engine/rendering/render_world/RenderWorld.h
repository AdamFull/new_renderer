#pragma once

#include <engine/rendering/render_world/RenderWorldInterface.h>

#include <memory>
#include <vector>

namespace engine
{
	namespace render
	{
		class IRenderer;

		class RenderWorld final : public IRenderWorld
		{
		public:
			RenderWorld(IRenderer& renderer);
			~RenderWorld() = default;

			uint32_t CreateRenderCamera() override;
			IRenderCamera& GetRenderCamera(uint32_t camera_id) override;
		private:
			IRenderer& renderer;

			std::vector<std::unique_ptr<IRenderCamera>> render_cameras{};
		};
	}
}