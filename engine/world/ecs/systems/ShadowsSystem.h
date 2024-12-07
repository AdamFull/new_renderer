#pragma once

#include <engine/rendering/RendererInterface.h>
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
				//utl::hash_map<render::ERenderGeometryCullerType, utl::vector<utl::vector<u64>>> m_shadowCullers{};
				render::IRenderWorld& render_world;
			};
		}
	}
}