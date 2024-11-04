#pragma once

#include <engine/rendering/RendererInterface.h>
#include <engine/world/ecs/systems/BaseSystem.h>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			class CMeshSystem : public ISystem
			{
			public:
				CMeshSystem(render::IRenderer& renderer);
				virtual ~CMeshSystem() override = default;

				void __Create() override;
				void __Update(f32 fDt) override;
			private:
				render::IRenderWorld& render_world;
			};
		}
	}
}