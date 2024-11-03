#include "RenderWorld.h"
#include <engine/rendering/Rendering.h>
#include <engine/rendering/DynamicBuffer.h>

#include "RenderCamera.h"

namespace engine
{
	namespace render
	{
		RenderWorld::RenderWorld(IRenderer& renderer) :
			renderer{ renderer }
		{
			auto& resource_context = renderer.GetResourceContext();

			node_buffer = std::make_unique<DynamicBuffer>(resource_context, sizeof(FSceneNode), 64ull, gfx::BufferUsageFlag::eStorageBuffer);
			matrices_buffer = std::make_unique<DynamicBuffer>(resource_context, sizeof(glm::mat4), 64ull, gfx::BufferUsageFlag::eStorageBuffer);

			frustum_buffer = std::make_unique<DynamicBuffer>(resource_context, sizeof(glm::vec4), 64ull, gfx::BufferUsageFlag::eStorageBuffer);
			camera_buffer = std::make_unique<DynamicBuffer>(resource_context, sizeof(FCamera), 64ull, gfx::BufferUsageFlag::eStorageBuffer);
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

		void RenderWorld::Flush()
		{
			// Update buffers on gpu


		}
	}
}