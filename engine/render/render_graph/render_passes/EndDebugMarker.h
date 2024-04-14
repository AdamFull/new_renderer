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

			void init(EQueueTypeFlagBits queue);
			void build(const FPassBuildInfo& build_info) override;
			void execute(f32 dt) override;

			EPassType type() override { return EPassType::eDebug; }
		};
	}
}