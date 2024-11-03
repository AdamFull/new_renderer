#include "RenderWorld.h"
#include <engine/rendering/RendererInterface.h>

#include "RenderCamera.h"

namespace engine
{
	namespace render
	{
		RenderWorld::RenderWorld(IRenderer& renderer) :
			renderer{ renderer }
		{

		}

		uint32_t RenderWorld::CreateRenderCamera()
		{
			render_cameras.emplace_back(std::make_unique<RenderCamera>());
			return render_cameras.size() - 1ull;
		}

		IRenderCamera& RenderWorld::GetRenderCamera(uint32_t camera_id)
		{
			return *render_cameras[camera_id];
		}
	}
}