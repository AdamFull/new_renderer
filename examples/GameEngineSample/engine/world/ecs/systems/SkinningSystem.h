#pragma once

#include "../../../rendering/RendererInterface.h"
#include "BaseSystem.h"

namespace engine::world::ecs {
	class SkinningSystem final : public ISystem {
	public:
		SkinningSystem(render::IRenderer& renderer);
		virtual ~SkinningSystem() override = default;

		void __create() override;
		void __update(float fDt) override;
	private:
		render::IRenderWorld& render_world;
	};
}