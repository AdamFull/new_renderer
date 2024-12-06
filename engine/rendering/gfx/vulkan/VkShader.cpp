#include "VkShader.h"
#include "VkDevice.h"
#include "VkDebug.h"

#include "VkUtil.h"

#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>

namespace engine
{
	namespace gfx
	{
		namespace vulkan
		{
			ShaderModule::ShaderModule(Device& device, const ShaderStageInfo& create_info) :
				VulkanResource{ nullptr, device.GetAllocationCallbacks() },
				device{ device }
			{
				vk::ShaderModuleCreateInfo vk_create_info{};
				vk_create_info.codeSize = static_cast<uint32_t>(create_info.code.size());
				vk_create_info.pCode = reinterpret_cast<const uint32_t*>(create_info.code.data());

				vk::Result result = Handle(device).createShaderModule(&vk_create_info, allocation_callbacks, &handle);

				std::strcpy(entry_point_name, create_info.entry_point);

				stage_create_info.pName = entry_point_name;
				stage_create_info.module = handle;
				stage_create_info.stage = ShaderStageFlagToVk(create_info.stage);
			}

			ShaderModule::~ShaderModule()
			{
				Handle(device).destroyShaderModule(handle, allocation_callbacks);
			}

			const vk::PipelineShaderStageCreateInfo& ShaderModule::GetStageCreateInfo() const
			{
				return stage_create_info;
			}


			VulkanShader::VulkanShader(Device& device, const ShaderCreateInfo& create_info) :
				device{ device }
			{
				static ShaderStageFlag all_stages[] =
				{
					ShaderStageFlag::eVertex, ShaderStageFlag::eTessellationControl, ShaderStageFlag::eTessellationEvaluation,
					ShaderStageFlag::eGeometry, ShaderStageFlag::eFragment, ShaderStageFlag::eCompute
				};

				for (int32_t i = 0; i < std::size(all_stages); ++i)
				{
					if (create_info.stages & all_stages[i])
					{
						auto& stage_create_info = create_info.shader_stages[i];
						auto& shader_module = shader_modules.NewElement();
						shader_module = std::make_unique<ShaderModule>(device, stage_create_info);
						AddReflectionStage(stage_create_info);
					}
				}
			}

			VulkanShader::VulkanShader(VulkanShader&& other) :
				shader_modules{ std::exchange(other.shader_modules, {}) },
				device{ other.device }
			{

			}

			void VulkanShader::SetResourceName(std::string_view name)
			{
				auto& debug_utils = device.GetDebugUtils();
				for (auto it = shader_modules.Begin(); it != shader_modules.End(); ++it)
				{
					auto& stage_ci = (*it)->GetStageCreateInfo();
					debug_utils.SetDebugName(stage_ci.module, std::format("{}[{}]", name, vk::to_string(stage_ci.stage)));
				}
			}

			const StaticArray<std::unique_ptr<ShaderModule>, 6ull>& VulkanShader::GetModules() const
			{
				return shader_modules;
			}

			bool HasSubstring(const std::string& str, const std::string& target)
			{
				return std::search(str.begin(), str.end(), target.begin(), target.end(), [](char a, char b) { return std::tolower(a) == std::tolower(b); }) != str.end();
			}

			auto FindShaderResourceByNameHash(ShaderReflection& reflection, uint32_t name_hash)
			{
				return std::find_if(reflection.shader_resources.Begin(), reflection.shader_resources.End(),
					[&name_hash](const ShaderResource& resource)
					{
						return resource.name_hash == name_hash;
					});
			}

			void CollectExecutionModes(ShaderReflection& reflection, spirv_cross::CompilerGLSL* compiler, uint32_t index)
			{
				auto mode = utl::cast<spv::ExecutionMode>(index);
				switch (mode)
				{
				case spv::ExecutionMode::ExecutionModeInvocations:
					reflection.invocation_count = compiler->get_execution_mode_argument(mode, 0);
					break;
				case spv::ExecutionMode::ExecutionModeLocalSize:
				{
					for (int32_t dim = 0; dim < 3; ++dim)
						reflection.local_sizes[dim] = compiler->get_execution_mode_argument(mode, dim);
				} break;
				case spv::ExecutionMode::ExecutionModeOutputVertices:
					reflection.output_vertex_count = compiler->get_execution_mode_argument(mode, 0);
					break;
				}
			}

			void CollectUniformBlockReflection(ShaderReflection& reflection, spirv_cross::CompilerGLSL* compiler, const spirv_cross::Resource& res, ShaderStageFlag stage, DescriptorType type)
			{
				uint32_t name_hash = utl::murmur_hash(res.name);
				auto found = FindShaderResourceByNameHash(reflection, name_hash);
				if (found != reflection.shader_resources.End())
				{
					found->stages |= stage;
					return;
				}

				if (HasSubstring(res.name, "global"))
					return;

				const auto& refl_type = compiler->get_type(res.type_id);
				auto& decoration = compiler->get_decoration_bitset(refl_type.self);
				uint32_t member_count = refl_type.member_types.size();

				ShaderResource& uniform_block = reflection.AddShaderResource();

				std::strcpy(uniform_block.name, res.name.c_str());
				uniform_block.name_hash = name_hash;
				uniform_block.set = compiler->get_decoration(res.id, spv::DecorationDescriptorSet);
				uniform_block.binding = compiler->get_decoration(res.id, spv::DecorationBinding);
				uniform_block.stages |= stage;
				uniform_block.type = type;
				uniform_block.size = compiler->get_declared_struct_size(refl_type);

				// Add internal variables if have them
				if (member_count > 0)
				{
					uniform_block.range_begin = reflection.shader_variables.count;

					for (int32_t i = 0; i < member_count; ++i)
					{
						uint64_t array_stride{ 0ull }, matrix_stride{ 0ull };

						auto& member_type = compiler->get_type(refl_type.member_types[i]);
						ShaderVariable& variable = reflection.AddShaderVariable();
						variable.size = compiler->get_declared_struct_member_size(refl_type, i);
						variable.offset = compiler->type_struct_member_offset(refl_type, i);

						auto& variable_name = compiler->get_member_name(refl_type.self, i);
						std::strcpy(variable.name, variable_name.c_str());
						variable.name_hash = utl::murmur_hash(variable_name);
					}

					uniform_block.variable_count = reflection.shader_variables.count;
				}
			}

			void CollectUniformReflection(ShaderReflection& reflection, spirv_cross::CompilerGLSL* compiler, const spirv_cross::Resource& res, ShaderStageFlag stage, DescriptorType type)
			{
				uint32_t name_hash = utl::murmur_hash(res.name);
				auto found = FindShaderResourceByNameHash(reflection, name_hash);
				if (found != reflection.shader_resources.End())
				{
					found->stages |= stage;
					return;
				}

				if (HasSubstring(res.name, "global"))
					return;

				ShaderResource& uniform = reflection.AddShaderResource();

				std::strcpy(uniform.name, res.name.c_str());
				uniform.name_hash = name_hash;
				uniform.set = compiler->get_decoration(res.id, spv::DecorationDescriptorSet);
				uniform.binding = compiler->get_decoration(res.id, spv::DecorationBinding);
				uniform.type = type;
				uniform.stages = stage;
			}

			void CollectConstantsReflection(ShaderReflection& reflection, spirv_cross::CompilerGLSL* compiler, const spirv_cross::Resource& res, ShaderStageFlag stage)
			{
				const auto& type = compiler->get_type(res.type_id);
				unsigned member_count = type.member_types.size();

				auto& constants = reflection.constants;

				if (!constants.name_hash)
				{
					std::strcpy(constants.name, res.name.c_str());
					constants.name_hash = utl::murmur_hash(res.name);
				}

				constants.size = std::max(constants.size, static_cast<uint32_t>(compiler->get_declared_struct_size(type)));
				constants.stages |= stage;

				if (member_count > 0)
				{
					constants.range_begin = reflection.shader_variables.count;

					for (int32_t i = 0; i < member_count; ++i)
					{
						uint64_t array_stride{ 0ull }, matrix_stride{ 0ull };

						auto& member_type = compiler->get_type(type.member_types[i]);
						ShaderVariable& variable = reflection.AddShaderVariable();
						variable.size = compiler->get_declared_struct_member_size(type, i);
						variable.offset = compiler->type_struct_member_offset(type, i);

						auto& variable_name = compiler->get_member_name(type.self, i);
						std::strcpy(variable.name, variable_name.c_str());
						variable.name_hash = utl::murmur_hash(variable_name);
					}

					constants.variable_count = reflection.shader_variables.count;
				}
			}

			void CollectVertexInputs(ShaderReflection& reflection, spirv_cross::CompilerGLSL* compiler, const spirv_cross::Resource& res)
			{
				VertexInput& vertex_input = reflection.AddVertexInput();

				std::strcpy(vertex_input.name, res.name.c_str());
				vertex_input.name_hash = utl::murmur_hash(res.name);
				vertex_input.location = compiler->get_decoration(res.id, spv::DecorationLocation);

				spirv_cross::SPIRType type = compiler->get_type(res.type_id);
				vertex_input.vec_size = type.vecsize;
				vertex_input.columns = type.columns;
				vertex_input.size = 4u * vertex_input.vec_size * vertex_input.columns;
			}

			void VulkanShader::AddReflectionStage(const ShaderStageInfo& create_info)
			{
				const uint32_t* code = reinterpret_cast<const uint32_t*>(create_info.code.data());
				uint32_t code_size{ static_cast<uint32_t>(create_info.code.size() / sizeof(uint32_t)) };

				spirv_cross::CompilerGLSL compiler(code, code_size);
				auto active = compiler.get_active_interface_variables();
				auto resources = compiler.get_shader_resources(active);
				compiler.set_enabled_interface_variables(std::move(active));

				// Parse execution modes
				auto& execution_modes = compiler.get_execution_mode_bitset();
				execution_modes.for_each_bit([&](u32 i) { CollectExecutionModes(reflection, &compiler, i); });

				// Parse uniform buffers
				for (const auto& res : resources.uniform_buffers)
				{
					CollectUniformBlockReflection(reflection, &compiler, res, create_info.stage, DescriptorType::eUniformBuffer);
				}

				// Parse storage buffers
				for (const auto& res : resources.storage_buffers)
				{
					CollectUniformBlockReflection(reflection, &compiler, res, create_info.stage, DescriptorType::eStorageBuffer);
				}

				// Parse push constants blocks
				for (const auto& res : resources.push_constant_buffers)
				{
					CollectConstantsReflection(reflection, &compiler, res, create_info.stage);
				}

				for (const auto& res : resources.sampled_images)
				{
					CollectUniformReflection(reflection, &compiler, res, create_info.stage, DescriptorType::eCombinedImageSampler);
				}

				for (const auto& res : resources.separate_images)
				{
					CollectUniformReflection(reflection, &compiler, res, create_info.stage, DescriptorType::eSampledImage);
				}

				for (const auto& res : resources.separate_samplers)
				{
					CollectUniformReflection(reflection, &compiler, res, create_info.stage, DescriptorType::eSampler);
				}

				for (const auto& res : resources.storage_images)
				{
					CollectUniformReflection(reflection, &compiler, res, create_info.stage, DescriptorType::eStorageImage);
				}

				for (const auto& res : resources.subpass_inputs)
				{
					CollectUniformReflection(reflection, &compiler, res, create_info.stage, DescriptorType::eInputAttachment);
				}

				if (create_info.stage == ShaderStageFlag::eVertex)
				{
					for (const auto& res : resources.stage_inputs)
					{
						CollectVertexInputs(reflection, &compiler, res);
					}
				}


				std::sort(reflection.shader_resources.Begin(), reflection.shader_resources.End(), [](const ShaderResource& a, const ShaderResource& b)
					{
						// First compare by set
						if (a.set != b.set)
						{
							return a.set < b.set;
						}

						// If sets are equal, compare by binding
						return a.binding < b.binding;
					});
			}
		}
	}
}