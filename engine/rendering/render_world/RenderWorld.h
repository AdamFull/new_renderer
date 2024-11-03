#pragma once

#include <engine/rendering/render_world/RenderWorldInterface.h>
#include <engine/rendering/DynamicBuffer.h>

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

			void Flush() override;
		private:
			IRenderer& renderer;

			std::vector<std::unique_ptr<IRenderCamera>> render_cameras{};

			std::unique_ptr<IDynamicBuffer> node_buffer{};
			std::unique_ptr<IDynamicBuffer> matrices_buffer{};

			std::unique_ptr<IDynamicBuffer> frustum_buffer{};
			std::unique_ptr<IDynamicBuffer> camera_buffer{};
		};
	}
}