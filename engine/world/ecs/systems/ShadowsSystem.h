#pragma once

#include <engine/interface/render/RenderInterface.h>
#include <engine/world/ecs/systems/BaseSystem.h>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			class CShadowsSystem : public ISystem
			{
			public:
				CShadowsSystem(CSceneGraph* scenegraph, render::IRenderInterface* render_interface);
				virtual ~CShadowsSystem() override = default;

				void __create() override;
				void __update(float fDt) override;
			private:
				utl::hash_map<render::ERenderGeometryCullerType, utl::vector<utl::vector<u64>>> m_shadowCullers{};
				render::IRenderInterface* m_pRenderInterface{ nullptr };
			};
		}
	}
}