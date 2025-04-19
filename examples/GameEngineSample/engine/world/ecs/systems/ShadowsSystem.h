#pragma once

#include "../../../rendering/RendererInterface.h"
#include "BaseSystem.h"

namespace engine::world::ecs {
	class ShadowsSystem final : public ISystem {
	public:
		ShadowsSystem(render::IRenderer& renderer);
		virtual ~ShadowsSystem() override = default;

		void __create() override;
		void __update(float fDt) override;
	private:
		//std::unordered_map<render::ERenderGeometryCullerType, std::vector<std::vector<uint64_t>>> m_shadowCullers{};
		render::IRenderWorld& render_world;
	};
}