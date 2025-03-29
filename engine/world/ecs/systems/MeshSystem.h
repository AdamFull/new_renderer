#pragma once

#include <engine/rendering/RendererInterface.h>
#include "BaseSystem.h"

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
				void __Update(float fDt) override;
			private:
				render::IRenderWorld& render_world;
			};
		}
	}
}