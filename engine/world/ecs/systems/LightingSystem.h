#pragma once

#include <engine/interface/render/RenderInterface.h>
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
				CLightingSystem(CSceneGraph* scenegraph, render::IRenderInterface* render_interface);
				virtual ~CLightingSystem() override = default;

				void __create() override;
				void __update(float fDt) override;
			private:
				render::IRenderInterface* m_pRenderInterface{ nullptr };
			};
		}
	}
}