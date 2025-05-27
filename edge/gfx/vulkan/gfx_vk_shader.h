#pragma once

#include "../gfx_base.h"
#include "gfx_vk_resource.h"

namespace edge::gfx::vulkan
{
	class Device;

	class ShaderModule final : public Resource<vk::ShaderModule> {
	public:
		ShaderModule(Device& devices, const ShaderStageInfo& create_info);
		~ShaderModule();

		ShaderModule(ShaderModule&& other) = delete;
		ShaderModule(const ShaderModule&) = delete;
		ShaderModule& operator=(const ShaderModule&) = delete;
		ShaderModule& operator=(ShaderModule&&) = delete;

		const vk::PipelineShaderStageCreateInfo& get_stage_create_info() const;
	private:
		Device& device;

		char entry_point_name[255];
		vk::PipelineShaderStageCreateInfo stage_create_info{};
	};

	class Shader final : public IShader {
	public:
		Shader(Device& device, std::span<const ShaderStageInfo> shader_stages, ShaderReflection* refl);
		Shader(Shader&& other);
		~Shader() = default;

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;
		Shader& operator=(Shader&&) = delete;

		void set_resource_name(std::string_view name) const noexcept override;

		auto get_modules() const noexcept -> std::span<const std::unique_ptr<ShaderModule>> {
			return shader_modules;
		}
	private:
		Device& device;
		utl::fixed_vector<std::unique_ptr<ShaderModule>, 6ull> shader_modules{};
	};
}