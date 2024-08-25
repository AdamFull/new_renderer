#pragma once 

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		// Stores global scene transformation
		class CRenderTransform : public IRenderTransform
		{
		public:
			void Create() override;
		private:
			u32 m_matrixBlock{ ~0u };
		};
	}
}