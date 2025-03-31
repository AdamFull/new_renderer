#pragma once

#include "../../../rendering/RendererInterface.h"
#include "BaseSystem.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			class CameraSystem : public ISystem
			{
			public:
				CameraSystem(render::IRenderer& renderer);
				virtual ~CameraSystem() override = default;

				void __Create() override;
				void __Update(float fDt) override;
			private:
				void OnConstruct(entt::registry& registry, entt::entity entity);
				void OnDestruct(entt::registry& registry, entt::entity entity);
			private:
				render::IRenderWorld& render_world;
			};
		}
	}
}