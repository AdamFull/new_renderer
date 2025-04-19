#pragma once

#include "../../../rendering/RendererInterface.h"
#include "BaseSystem.h"
#include "../components/fwd.h"

namespace engine::world::ecs {
	class ScenegraphSystem final : public ISystem
	{
	public:
		ScenegraphSystem(render::IRenderer& renderer);
		virtual ~ScenegraphSystem() override = default;

		void __create() override;
		void __update(float fDt) override;
	private:
		render::IRenderWorld& render_world;
	private:
		static void build_hierarchy(entt::entity node);
		static void init_matrix(TransformComponent* transform);
		static void init_matrices(entt::registry* registry, entt::entity node);
		static void calculate_matrices(entt::registry* registry, entt::entity node);
	};
}