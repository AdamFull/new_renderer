#pragma once

#include "../../../rendering/RendererInterface.h"
#include "BaseSystem.h"

namespace engine::world::ecs {
	class LightingSystem final : public ISystem {
	public:
		LightingSystem(render::IRenderer& renderer);
		virtual ~LightingSystem() override = default;

		void __create() override;
		void __update(float fDt) override;
	private:
		void on_construct_directional_light(entt::registry& registry, entt::entity entity);
		void on_destruct_directional_light(entt::registry& registry, entt::entity entity);

		void on_construct_pointl_light(entt::registry& registry, entt::entity entity);
		void on_destruct_pointl_light(entt::registry& registry, entt::entity entity);

		void on_construct_spot_light(entt::registry& registry, entt::entity entity);
		void on_destruct_spot_light(entt::registry& registry, entt::entity entity);
	private:
		render::IRenderWorld& render_world;
	};
}