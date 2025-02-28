#pragma once

#include "../gfx_base.h"
#include "VkResource.h"

namespace edge
{
	namespace gfx
	{
		namespace vulkan
		{
			class Device;

			class ShaderModule final : public VulkanResource<vk::ShaderModule>
			{
			public:
				ShaderModule(Device& devices, const ShaderStageInfo& create_info);
				~ShaderModule();

				ShaderModule(ShaderModule&& other) = delete;
				ShaderModule(const ShaderModule&) = delete;
				ShaderModule& operator=(const ShaderModule&) = delete;
				ShaderModule& operator=(ShaderModule&&) = delete;

				const vk::PipelineShaderStageCreateInfo& GetStageCreateInfo() const;
			private:
				Device& device;

				char entry_point_name[255];
				vk::PipelineShaderStageCreateInfo stage_create_info{};
			};

			class VulkanShader final : public IShader
			{
			public:
				VulkanShader(Device& device, std::span<const ShaderStageInfo> shader_stages);
				VulkanShader(VulkanShader&& other);
				~VulkanShader() = default;

				VulkanShader(const VulkanShader&) = delete;
				VulkanShader& operator=(const VulkanShader&) = delete;
				VulkanShader& operator=(VulkanShader&&) = delete;

				void SetResourceName(std::string_view name) const noexcept override;

				auto GetModules() const noexcept -> StaticArray<std::unique_ptr<ShaderModule>, 6ull> const&
				{
					return shader_modules;
				}
			private:
				void AddReflectionStage(const ShaderStageInfo& create_info);
			private:
				Device& device;
				StaticArray<std::unique_ptr<ShaderModule>, 6ull> shader_modules{};
			};
		}
	}
}