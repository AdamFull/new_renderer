#pragma once

#include <engine/rendering/RendererInterface.h>
#include <engine/world/ecs/systems/BaseSystem.h>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			class CLightingSystem : public ISystem
			{
			public:
				CLightingSystem(render::IRenderer& renderer);
				virtual ~CLightingSystem() override = default;

				void __Create() override;
				void __Update(float fDt) override;
			private:
				render::IRenderWorld& render_world;
			};
		}
	}
}