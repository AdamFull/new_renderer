#pragma once

#if USE_NSIGHT_AFTERMATH

#include "../gfx_nsight_aftermath.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class VkNsightAftermath final : public NsightAftermath
			{
			public:
				VkNsightAftermath();
				~VkNsightAftermath() = default;

				void RegisterShaderBinary(std::span<uint8_t> shader_code) override;
				void RegisterShaderDebugBinary(std::span<uint8_t> stripped_shader_code, std::span<uint8_t> full_shader_code) override;
			private:
				void SetMarkerImpl(ICommandList* cmd_list, const void* data_index) override;
			};
		}
	}
}

#endif