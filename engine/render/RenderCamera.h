#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CRenderCamera : public IRenderCamera
		{
		public:
			CRenderCamera(IRenderInterface* render_interface);
			~CRenderCamera() = default;

			void Create() override;

		private:
			IRenderWorld* m_pRenderWorld{ nullptr };

			u32 m_cameraId{ ~0u };
			u32 m_frustumId{ ~0u };
			u32 m_matricesId{ ~0u };
		};
	}
}