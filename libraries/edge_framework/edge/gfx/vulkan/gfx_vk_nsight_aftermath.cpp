#include <vulkan/vulkan.hpp>

#include "gfx_vk_command_list.h"
#include "gfx_vk_nsight_aftermath.h"

#if USE_NSIGHT_AFTERMATH

namespace edge::gfx::vulkan
{
	VkNsightAftermath::VkNsightAftermath() :
		NsightAftermath() {
	}

	void VkNsightAftermath::register_shader_binary(std::span<uint8_t> shader_code) {
		// Create shader hash for the shader
		const GFSDK_Aftermath_SpirvCode shader{ shader_code.data(), uint32_t(shader_code.size()) };
		GFSDK_Aftermath_ShaderBinaryHash shader_hash;

		GFSDK_Aftermath_GetShaderHashSpirv(GFSDK_Aftermath_Version_API, &shader, &shader_hash);

		shader_binary_map[shader_hash] = shader_code;
	}

	void VkNsightAftermath::register_shader_debug_binary(std::span<uint8_t> stripped_shader_code, std::span<uint8_t> full_shader_code) {
		// Generate shader debug name.
		GFSDK_Aftermath_ShaderDebugName debugName;
		const GFSDK_Aftermath_SpirvCode shader{ full_shader_code.data(), uint32_t(full_shader_code.size()) };
		const GFSDK_Aftermath_SpirvCode strippedShader{ stripped_shader_code.data(), uint32_t(stripped_shader_code.size()) };
		GFSDK_Aftermath_GetShaderDebugNameSpirv(GFSDK_Aftermath_Version_API, &shader, &strippedShader, &debugName);

		// Store the data for shader instruction address mapping when decoding GPU crash dumps.
		shader_full_binary_map[debugName] = full_shader_code;
	}

	void VkNsightAftermath::set_marker_impl(ICommandList* cmd_list, const void* data_idx) {
		auto* vk_cmd_list = static_cast<VulkanCommandList*>(cmd_list);
		auto& handle = vk_cmd_list->GetHandle();
		handle.setCheckpointNV(data_idx);
	}
}

#endif