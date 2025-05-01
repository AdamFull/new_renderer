#include "gfx_vk_shader.h"
#include "gfx_vk_device.h"
#include "gfx_vk_debug.h"

#include "gfx_vk_util.h"

//#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>

namespace edge::gfx::vulkan
{
	ShaderModule::ShaderModule(Device& device, const ShaderStageInfo& create_info) :
		Resource{ nullptr, device.get_allocation_callbacks() },
		device{ device } {

		vk::ShaderModuleCreateInfo vk_create_info{};
		vk_create_info.codeSize = static_cast<uint32_t>(create_info.code.size());
		vk_create_info.pCode = reinterpret_cast<const uint32_t*>(create_info.code.data());

		vk::Result result = device.get_handle().createShaderModule(&vk_create_info, allocation_callbacks, &handle_);

		std::strcpy(entry_point_name, create_info.entry_point);

		stage_create_info.pName = entry_point_name;
		stage_create_info.module = handle_;
		stage_create_info.stage = shader_stage_flag_to_vk(create_info.stage);
	}

	ShaderModule::~ShaderModule() {
		device.get_handle().destroyShaderModule(handle_, allocation_callbacks);
	}

	const vk::PipelineShaderStageCreateInfo& ShaderModule::get_stage_create_info() const {
		return stage_create_info;
	}


	Shader::Shader(Device& device, std::span<const ShaderStageInfo> shader_stages, ShaderReflection* refl) :
		device{ device } {

		for (const auto& stage : shader_stages) {
			auto& new_shader_module = shader_modules.new_element();
			new_shader_module = std::make_unique<ShaderModule>(device, stage);
			//add_reflection_stage(stage);
		}

		if (refl) {
			memcpy(&reflection, refl, sizeof(ShaderReflection));
		}
	}

	Shader::Shader(Shader&& other) :
		shader_modules{ std::exchange(other.shader_modules, {}) },
		device{ other.device } {
	}

	void Shader::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();
		for (auto it = shader_modules.begin(); it != shader_modules.end(); ++it) {
			auto& stage_ci = (*it)->get_stage_create_info();
			debug_utils.set_debug_name(stage_ci.module, std::format("{}[{}]", name, vk::to_string(stage_ci.stage)));
		}
	}
}