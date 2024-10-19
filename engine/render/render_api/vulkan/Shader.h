#pragma once

#include <engine/render/render_api/vulkan/VkResourceStructures.h>
#include <engine/render/render_api/vulkan/VkShaderReflection.h>
#include <engine/render/render_api/vulkan/Pipeline.h>

namespace engine
{
	namespace render
	{
		namespace vulkan
		{
			class RenderDevice;
			class Buffer;
			class Shader;
			class DescriptorSetLayout;
			class Pipeline;

			class ShaderBuilder final
			{
			public:
				CVkShaderReflectionBuilder reflection_builder{};

				vk::PipelineBindPoint bind_point{};
				uint32_t usage_count{ 1u };

				struct ShaderModuleInfo
				{
					vk::ShaderStageFlagBits stage{};
					vk::ShaderModuleCreateInfo create_info{};
					const char* entry_point{ "main" };
				};

				std::array<ShaderModuleInfo, 8ull> shader_modules{};
				uint32_t shader_module_count{ 0u };

				std::array<vk::DescriptorSetLayout, 4ull> default_layouts{};
				uint32_t default_descriptor_count{ 0u };

				mutable GraphicsPipelineBuilder graphics_pipeline_builder{};
				mutable ComputePipelineBuilder compute_pipeline_builder{};

				ShaderBuilder& SetBindPoint(vk::PipelineBindPoint bind_point);
				ShaderBuilder& SetUsageCount(uint32_t usage_count);
				ShaderBuilder& AddShaderModule(vk::ShaderStageFlagBits stage, std::span<uint32_t> code);
				ShaderBuilder& AddDefaultLayout(vk::DescriptorSetLayout layout);

				Shader Build(RenderDevice& render_device);
				std::unique_ptr<Shader> BuildUnique(RenderDevice& render_device);
			};

			class Shader final
			{
			public:
				static constexpr size_t k_max_shader_module_handles{ 8ull };

				Shader(RenderDevice& render_device, ShaderBuilder& builder);
				~Shader();

				Shader(Shader const&) = delete;
				Shader(Shader&& other) noexcept;

				Shader& operator=(const Shader&) = delete;
				Shader& operator=(Shader&&) = default;

				void BindDescriptors(vk::CommandBuffer& command_buffer, vk::DescriptorSet global_descriptor);

			public:
				RenderDevice& render_device;

				// Pipeline object
				std::unique_ptr<Pipeline> pipeline{};
				vk::PipelineBindPoint bind_point{};

				std::array<vk::ShaderModule, k_max_shader_module_handles> shader_modules{};
				uint32_t shader_module_count{ 0u };

				// Root signature
				vk::PipelineLayout layout{};
				std::array<std::unique_ptr<DescriptorSetLayout>, k_max_descriptor_sets> descriptor_set_layouts{};
				std::array<vk::DescriptorSet, k_max_descriptor_sets> descriptor_sets{};
				utl::vector<vk::PushConstantRange> pushConstantRanges{};

				// Other...
				utl::hash_map<u32, u32> descriptorUsage{};

				utl::hash_map<u64, std::unique_ptr<Buffer>> uniform_buffers{};
				u64 maximumDescriptorSlots{ 0ull };

				FVkShaderReflectionData reflection{};
			};
		}
	}
}