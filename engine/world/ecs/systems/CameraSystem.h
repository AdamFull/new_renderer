#pragma once

#include <engine/interface/render/RenderInterface.h>
#include <engine/world/ecs/systems/BaseSystem.h>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			class CCameraSystem : public ISystem
			{
			public:
				CCameraSystem(CSceneGraph* scenegraph, render::IRenderInterface* render_interface);
				virtual ~CCameraSystem() override = default;

				void __create() override;
				void __update(float fDt) override;
			private:
				render::IRenderInterface* m_pRenderInterface{ nullptr };
			};
		}
	}
}