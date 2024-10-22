#include <VkShader.h>
#include <VkDevice.h>

#include <VkUtil.h>

#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>

namespace gfx
{
	namespace vulkan
	{
		ShaderModule::ShaderModule(Device& device, const ShaderStageInfo& create_info) :
			VulkanResource{ nullptr, device.GetAllocationCallbacks() },
			device{ device }
		{
			vk::ShaderModuleCreateInfo vk_create_info{};
			vk_create_info.codeSize = static_cast<uint32_t>(create_info.code.size() / sizeof(uint32_t));
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


		VulkanShader::VulkanShader(Device& device, const ShaderCreateInfo& create_info)
		{
			if (create_info.stages & ShaderStageFlag::eVertex)
			{
				auto& stage_create_info = create_info.shader_stages[0];
				// TODO: Validate that it's correct stage

				shader_modules[shader_module_count++] = std::make_unique<ShaderModule>(device, stage_create_info);
				AddReflectionStage(stage_create_info);
			}
		}

		VulkanShader::VulkanShader(VulkanShader&& other) :
			shader_modules{ std::exchange(other.shader_modules, {}) },
			shader_module_count{ std::exchange(other.shader_module_count, {}) }
		{

		}

		//void build_uniform_block_reflection(FVkShaderReflectionData& reflection_data, spirv_cross::CompilerGLSL* compiler, const spirv_cross::Resource& res, vk::ShaderStageFlagBits shaderStage, vk::DescriptorType descriptorType)
		//{
		//	u64 name_hash = utl::murmur_hash_64(res.name);
		//	auto it = reflection_data.uniformBlocks.find(name_hash);
		//	if (it != reflection_data.uniformBlocks.end())
		//	{
		//		it->second.stageFlags |= shaderStage;
		//		return;
		//	}
		//
		//	if (has_substring(res.name, "global"))
		//		return;
		//
		//	const auto& type = compiler->get_type(res.type_id);
		//	auto& decoration = compiler->get_decoration_bitset(type.self);
		//	unsigned member_count = type.member_types.size();
		//
		//	FVkReflectionUniformBlock uniform_block{};
		//	uniform_block.set = compiler->get_decoration(res.id, spv::DecorationDescriptorSet);
		//	uniform_block.binding = compiler->get_decoration(res.id, spv::DecorationBinding);
		//
		//	for (u32 i = 0u; i < member_count; ++i)
		//	{
		//		u64 array_stride{ 0ull }, matrix_stride{ 0ull };
		//		auto& member_type = compiler->get_type(type.member_types[i]);
		//
		//		FVkReflectionUniform uniform{};
		//		uniform.size = compiler->get_declared_struct_member_size(type, i);
		//		uniform.offset = compiler->type_struct_member_offset(type, i);
		//		if (!member_type.array.empty() && uniform.size == 0)
		//			array_stride = compiler->type_struct_member_array_stride(type, i);
		//
		//		if (member_type.columns > 1)
		//			matrix_stride = compiler->type_struct_member_matrix_stride(type, i);
		//
		//		uniform.stageFlags = shaderStage;
		//
		//		u64 member_name_hash = utl::murmur_hash_64(compiler->get_member_name(type.self, i));
		//		uniform_block.uniforms.emplace(member_name_hash, uniform);
		//	}
		//
		//	uniform_block.descriptorType = descriptorType;
		//	uniform_block.size = compiler->get_declared_struct_size(type);
		//	uniform_block.stageFlags = shaderStage;
		//
		//	reflection_data.uniformBlocks.emplace(name_hash, uniform_block);
		//}

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

		void VulkanShader::AddReflectionStage(const ShaderStageInfo& create_info)
		{
			const uint32_t* code = reinterpret_cast<const uint32_t*>(create_info.code.data());
			uint32_t code_size{ static_cast<uint32_t>(create_info.code.size() / sizeof(uint32_t)) };

			spirv_cross::CompilerGLSL compiler(code, code_size);
			auto active = compiler.get_active_interface_variables();
			auto resources = compiler.get_shader_resources(active);
			compiler.set_enabled_interface_variables(std::move(active));

			// Parse execution modes
			//auto& execution_modes = compiler.get_execution_mode_bitset();
			//execution_modes.for_each_bit([&](u32 i) { parse_execution_mode(reflectionData, &compiler, i); });

			// Parse uniform buffers
			for (const auto& res : resources.uniform_buffers)
				CollectUniformBlockReflection(reflection, &compiler, res, create_info.stage, DescriptorType::eUniformBuffer);

			// Parse storage buffers
			for (const auto& res : resources.storage_buffers)
				CollectUniformBlockReflection(reflection, &compiler, res, create_info.stage, DescriptorType::eStorageBuffer);
		}
	}
}