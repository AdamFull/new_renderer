#pragma once

#include <engine/rendering/RendererInterface.h>
#include "BaseSystem.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			class LightingSystem : public ISystem
			{
			public:
				LightingSystem(render::IRenderer& renderer);
				virtual ~LightingSystem() override = default;

				void __Create() override;
				void __Update(float fDt) override;
			private:
				void OnConstructDirectionalLight(entt::registry& registry, entt::entity entity);
				void OnDestructDirectionalLight(entt::registry& registry, entt::entity entity);

				void OnConstructPointlLight(entt::registry& registry, entt::entity entity);
				void OnDestructPointlLight(entt::registry& registry, entt::entity entity);

				void OnConstructSpotLight(entt::registry& registry, entt::entity entity);
				void OnDestructSpotLight(entt::registry& registry, entt::entity entity);
			private:
				render::IRenderWorld& render_world;
			};
		}
	}
}