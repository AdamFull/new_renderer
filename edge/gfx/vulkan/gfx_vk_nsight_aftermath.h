#pragma once

#if USE_NSIGHT_AFTERMATH

#include "../gfx_nsight_aftermath.h"

namespace edge::gfx::vulkan
{
	class VkNsightAftermath final : public NsightAftermath {
	public:
		VkNsightAftermath();
		~VkNsightAftermath() = default;

		void register_shader_binary(std::span<uint8_t> shader_code) override;
		void register_shader_debug_binary(std::span<uint8_t> stripped_shader_code, std::span<uint8_t> full_shader_code) override;
	private:
		void set_marker_impl(ICommandList* cmd_list, const void* data_index) override;
	};
}

#endif