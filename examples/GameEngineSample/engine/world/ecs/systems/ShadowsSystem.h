#pragma once

#include "../../../rendering/RendererInterface.h"
#include "BaseSystem.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			class CShadowsSystem : public ISystem
			{
			public:
				CShadowsSystem(render::IRenderer& renderer);
				virtual ~CShadowsSystem() override = default;

				void __Create() override;
				void __Update(float fDt) override;
			private:
				//std::unordered_map<render::ERenderGeometryCullerType, std::vector<std::vector<uint64_t>>> m_shadowCullers{};
				render::IRenderWorld& render_world;
			};
		}
	}
}