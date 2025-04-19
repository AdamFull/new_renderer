#pragma once

#include "../../../rendering/RendererInterface.h"
#include "BaseSystem.h"

namespace engine::world::ecs {
	class CameraSystem final : public ISystem {
	public:
		CameraSystem(render::IRenderer& renderer);
		virtual ~CameraSystem() override = default;

		void __create() override;
		void __update(float fDt) override;
	private:
		void on_construct(entt::registry& registry, entt::entity entity);
		void on_destruct(entt::registry& registry, entt::entity entity);
	private:
		render::IRenderWorld& render_world;
	};
}