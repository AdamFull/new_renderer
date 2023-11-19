#pragma once

#include <interface/render/RenderInterface.h>
#include <world/ecs/systems/BaseSystem.h>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			class CMeshSystem : public ISystem
			{
			public:
				CMeshSystem(CSceneGraph* scenegraph, render::IRenderInterface* render_interface);
				virtual ~CMeshSystem() override = default;

				void __create() override;
				void __update(float fDt) override;
			private:
				render::IRenderInterface* m_pRenderInterface{ nullptr };
			};
		}
	}
}