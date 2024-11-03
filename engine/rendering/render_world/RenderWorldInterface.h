#pragma once

#include <engine/rendering/render_world/RenderCameraInterface.h>

namespace engine
{
	namespace render
	{
		class IRenderWorld
		{
		public:
			virtual ~IRenderWorld() = default;

			virtual uint32_t CreateRenderCamera() = 0;
			virtual IRenderCamera& GetRenderCamera(uint32_t camera_id) = 0;

			virtual void AddGeometryDraw() = 0;

			virtual void Flush() = 0;
		};
	}
}