#pragma once

#include "BasePass.h"

namespace engine
{
	namespace render
	{
		class CEndDebugMarker : public IBasePass
		{
		public:
			CEndDebugMarker(IRenderInterface* render_interface);

			~CEndDebugMarker() = default;

			void Init(EQueueTypeFlagBits queue);
			void Build(const FPassBuildInfo& build_info) override;
			void Execute(f32 dt) override;

			EPassType Type() override { return EPassType::eDebug; }
		};
	}
}