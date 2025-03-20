#include "VkShader.h"
#include "VkDevice.h"
#include "VkDebug.h"

#include "VkUtil.h"

#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>

namespace edge::gfx::vulkan
{
	ShaderModule::ShaderModule(Device& device, const ShaderStageInfo& create_info) :
		VulkanResource{ nullptr, device.get_allocation_callbacks() },
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


	VulkanShader::VulkanShader(Device& device, std::span<const ShaderStageInfo> shader_stages) :
		device{ device } {

		for (const auto& stage : shader_stages) {
			auto& new_shader_module = shader_modules.new_element();
			new_shader_module = std::make_unique<ShaderModule>(device, stage);
			add_reflection_stage(stage);
		}
	}

	VulkanShader::VulkanShader(VulkanShader&& other) :
		shader_modules{ std::exchange(other.shader_modules, {}) },
		device{ other.device } {
	}

	void VulkanShader::set_resource_name(std::string_view name) const noexcept {
		auto& debug_utils = device.get_debug_utils();
		for (auto it = shader_modules.begin(); it != shader_modules.end(); ++it) {
			auto& stage_ci = (*it)->get_stage_create_info();
			debug_utils.set_debug_name(stage_ci.module, std::format("{}[{}]", name, vk::to_string(stage_ci.stage)));
		}
	}

	bool has_substring(const std::string& str, const std::string& target) {
		return std::search(str.begin(), str.end(), target.begin(), target.end(), 
			[](char a, char b) { return std::tolower(a) == std::tolower(b); }) != str.end();
	}

	auto find_shader_resource_by_name_hash(ShaderReflection& reflection, uint32_t name_hash) {
		return std::find_if(reflection.shader_resources.begin(), reflection.shader_resources.end(),
			[&name_hash](const ShaderResource& resource) {
				return resource.name_hash == name_hash;
			});
	}

	void collect_execution_modes(ShaderReflection& reflection, spirv_cross::CompilerGLSL* compiler, uint32_t index) {
		auto mode = static_cast<spv::ExecutionMode>(index);
		switch (mode) {
		case spv::ExecutionMode::ExecutionModeInvocations:
			reflection.invocation_count = compiler->get_execution_mode_argument(mode, 0);
			break;
		case spv::ExecutionMode::ExecutionModeLocalSize: {
			for (int32_t dim = 0; dim < 3; ++dim)
				reflection.local_sizes[dim] = compiler->get_execution_mode_argument(mode, dim);
		} break;
		case spv::ExecutionMode::ExecutionModeOutputVertices:
			reflection.output_vertex_count = compiler->get_execution_mode_argument(mode, 0);
			break;
		}
	}

	void collect_uniform_block_reflection(ShaderReflection& reflection, spirv_cross::CompilerGLSL* compiler, 
		const spirv_cross::Resource& res, ShaderStageFlag stage, DescriptorType type) {

		uint32_t name_hash = murmur_hash(res.name);
		auto found = find_shader_resource_by_name_hash(reflection, name_hash);
		if (found != reflection.shader_resources.end())
		{
			found->stages |= stage;
			return;
		}

		if (has_substring(res.name, "global"))
			return;

		const auto& refl_type = compiler->get_type(res.type_id);
		auto& decoration = compiler->get_decoration_bitset(refl_type.self);
		uint32_t member_count = refl_type.member_types.size();

		ShaderResource& uniform_block = reflection.add_shader_resource();

		std::strcpy(uniform_block.name, res.name.c_str());
		uniform_block.name_hash = name_hash;
		uniform_block.set = compiler->get_decoration(res.id, spv::DecorationDescriptorSet);
		uniform_block.binding = compiler->get_decoration(res.id, spv::DecorationBinding);
		uniform_block.stages |= stage;
		uniform_block.type = type;
		uniform_block.size = compiler->get_declared_struct_size(refl_type);

		// Add internal variables if have them
		if (member_count > 0) {
			uniform_block.range_begin = reflection.shader_variables.count;

			for (int32_t i = 0; i < member_count; ++i) {
				uint64_t array_stride{ 0ull }, matrix_stride{ 0ull };

				auto& member_type = compiler->get_type(refl_type.member_types[i]);
				ShaderVariable& variable = reflection.add_shader_variable();
				variable.size = compiler->get_declared_struct_member_size(refl_type, i);
				variable.offset = compiler->type_struct_member_offset(refl_type, i);

				auto& variable_name = compiler->get_member_name(refl_type.self, i);
				std::strcpy(variable.name, variable_name.c_str());
				variable.name_hash = murmur_hash(variable_name);
			}

			uniform_block.variable_count = reflection.shader_variables.count;
		}
	}

	void collect_uniform_reflection(ShaderReflection& reflection, spirv_cross::CompilerGLSL* compiler, const spirv_cross::Resource& res, ShaderStageFlag stage, DescriptorType type) {
		uint32_t name_hash = murmur_hash(res.name);
		auto found = find_shader_resource_by_name_hash(reflection, name_hash);
		if (found != reflection.shader_resources.end()) {
			found->stages |= stage;
			return;
		}

		if (has_substring(res.name, "global"))
			return;

		ShaderResource& uniform = reflection.add_shader_resource();

		std::strcpy(uniform.name, res.name.c_str());
		uniform.name_hash = name_hash;
		uniform.set = compiler->get_decoration(res.id, spv::DecorationDescriptorSet);
		uniform.binding = compiler->get_decoration(res.id, spv::DecorationBinding);
		uniform.type = type;
		uniform.stages = stage;
	}

	void collect_constants_reflection(ShaderReflection& reflection, spirv_cross::CompilerGLSL* compiler, const spirv_cross::Resource& res, ShaderStageFlag stage) {
		const auto& type = compiler->get_type(res.type_id);
		unsigned member_count = type.member_types.size();

		auto& constants = reflection.constants;

		if (!constants.name_hash) {
			std::strcpy(constants.name, res.name.c_str());
			constants.name_hash = murmur_hash(res.name);
		}

		constants.size = std::max(constants.size, static_cast<uint32_t>(compiler->get_declared_struct_size(type)));
		constants.stages |= stage;

		if (member_count > 0) {
			constants.range_begin = reflection.shader_variables.count;

			for (int32_t i = 0; i < member_count; ++i) {
				uint64_t array_stride{ 0ull }, matrix_stride{ 0ull };

				auto& member_type = compiler->get_type(type.member_types[i]);
				ShaderVariable& variable = reflection.add_shader_variable();
				variable.size = compiler->get_declared_struct_member_size(type, i);
				variable.offset = compiler->type_struct_member_offset(type, i);

				auto& variable_name = compiler->get_member_name(type.self, i);
				std::strcpy(variable.name, variable_name.c_str());
				variable.name_hash = murmur_hash(variable_name);
			}

			constants.variable_count = reflection.shader_variables.count;
		}
	}

	void collect_vertex_inputs(ShaderReflection& reflection, spirv_cross::CompilerGLSL* compiler, const spirv_cross::Resource& res) {
		VertexInput& vertex_input = reflection.add_vertex_input();

		std::strcpy(vertex_input.name, res.name.c_str());
		vertex_input.name_hash = murmur_hash(res.name);
		vertex_input.location = compiler->get_decoration(res.id, spv::DecorationLocation);

		spirv_cross::SPIRType type = compiler->get_type(res.type_id);
		vertex_input.vec_size = type.vecsize;
		vertex_input.columns = type.columns;
		vertex_input.size = 4u * vertex_input.vec_size * vertex_input.columns;
	}

	void VulkanShader::add_reflection_stage(const ShaderStageInfo& create_info) {
		const uint32_t* code = reinterpret_cast<const uint32_t*>(create_info.code.data());
		uint32_t code_size{ static_cast<uint32_t>(create_info.code.size() / sizeof(uint32_t)) };

		spirv_cross::CompilerGLSL compiler(code, code_size);
		auto active = compiler.get_active_interface_variables();
		auto resources = compiler.get_shader_resources(active);
		compiler.set_enabled_interface_variables(std::move(active));

		// Parse execution modes
		auto& execution_modes = compiler.get_execution_mode_bitset();
		execution_modes.for_each_bit([&](uint32_t i) { collect_execution_modes(reflection, &compiler, i); });

		// Parse uniform buffers
		for (const auto& res : resources.uniform_buffers) {
			collect_uniform_block_reflection(reflection, &compiler, res, create_info.stage, DescriptorType::eUniformBuffer);
		}

		// Parse storage buffers
		for (const auto& res : resources.storage_buffers) {
			collect_uniform_block_reflection(reflection, &compiler, res, create_info.stage, DescriptorType::eStorageBuffer);
		}

		// Parse push constants blocks
		for (const auto& res : resources.push_constant_buffers) {
			collect_constants_reflection(reflection, &compiler, res, create_info.stage);
		}

		for (const auto& res : resources.sampled_images) {
			collect_uniform_reflection(reflection, &compiler, res, create_info.stage, DescriptorType::eCombinedImageSampler);
		}

		for (const auto& res : resources.separate_images) {
			collect_uniform_reflection(reflection, &compiler, res, create_info.stage, DescriptorType::eSampledImage);
		}

		for (const auto& res : resources.separate_samplers) {
			collect_uniform_reflection(reflection, &compiler, res, create_info.stage, DescriptorType::eSampler);
		}

		for (const auto& res : resources.storage_images) {
			collect_uniform_reflection(reflection, &compiler, res, create_info.stage, DescriptorType::eStorageImage);
		}

		for (const auto& res : resources.subpass_inputs) {
			collect_uniform_reflection(reflection, &compiler, res, create_info.stage, DescriptorType::eInputAttachment);
		}

		if (create_info.stage == ShaderStageFlag::eVertex) {
			for (const auto& res : resources.stage_inputs) {
				collect_vertex_inputs(reflection, &compiler, res);
			}
		}


		std::sort(reflection.shader_resources.begin(), reflection.shader_resources.end(), 
			[](const ShaderResource& a, const ShaderResource& b) {
				// First compare by set
				if (a.set != b.set) {
					return a.set < b.set;
				}

				// If sets are equal, compare by binding
				return a.binding < b.binding;
			});
	}
}