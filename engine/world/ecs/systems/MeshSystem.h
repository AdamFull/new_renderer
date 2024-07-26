#pragma once

#include <engine/interface/render/RenderInterface.h>
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
				CMeshSystem(render::IRenderInterface* render_interface);
				virtual ~CMeshSystem() override = default;

				void __Create() override;
				void __Update(f32 fDt) override;
			private:
				render::IRenderInterface* m_pRenderInterface{ nullptr };
			};
		}
	}
}