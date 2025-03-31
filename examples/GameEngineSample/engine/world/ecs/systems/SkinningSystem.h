#pragma once

#include "../../../rendering/RendererInterface.h"
#include "BaseSystem.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			class CSkinningSystem : public ISystem
			{
			public:
				CSkinningSystem(render::IRenderer& renderer);
				virtual ~CSkinningSystem() override = default;

				void __Create() override;
				void __Update(float fDt) override;
			private:
				render::IRenderWorld& render_world;
			};
		}
	}
}