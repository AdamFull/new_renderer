#pragma once

#include "../../../rendering/RendererInterface.h"
#include "BaseSystem.h"

namespace engine::world::ecs {
	class MeshSystem final : public ISystem {
	public:
		MeshSystem(render::IRenderer& renderer);
		virtual ~MeshSystem() override = default;

		void __create() override;
		void __update(float fDt) override;
	private:
		render::IRenderWorld& render_world;
	};
}