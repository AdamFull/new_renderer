#pragma once

#include "BasePass.h"

namespace engine
{
	namespace render
	{
		class CBeginDebugMarker : public IBasePass
		{
		public:
			CBeginDebugMarker(IRenderInterface* render_interface);

			~CBeginDebugMarker() = default;

			void init(const std::string& name, u32 color, EQueueTypeFlagBits queue);
			void build(const FPassBuildInfo& build_info) override;
			void execute(f32 dt) override;

			EPassType type() override { return EPassType::eDebug; }
		private:
			std::string m_markerName{};
			u32 m_markerColor{ 0xffffffff };
		};
	}
}