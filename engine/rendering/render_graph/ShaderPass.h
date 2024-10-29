#pragma once

#include <engine/rendering/ExecutionContext.h>
#include <engine/rendering/gfx/Base.h>

#include <algorithm>

namespace engine
{
	namespace gfx
	{
		class Context;
	}

	namespace render
	{
		enum class ShaderPassType
		{
			eNone,
			eCompute,
			eGraphics
		};

		struct PassCreateInfo
		{
			gfx::Shader* shader{ nullptr };
			gfx::PipelineCache* pipeline_cache{ nullptr };
			gfx::SharedDescriptor* shared_descriptor{ nullptr };

			std::string debug_name{};
			bool one_time{ false };
		};

		struct ComputePassCreateInfo : PassCreateInfo
		{
			bool divide_by_locals{ false };
			gfx::Texture* texture_size_target{ nullptr };
			uint32_t texture_subresource_index{ ~0u };

			gfx::Buffer* buffer_size_target{ nullptr };
		};

		struct RenderPassCreateInfo : PassCreateInfo
		{
			gfx::PipelineState state{};
		};

		struct ShaderResourceBinding
		{
			uint32_t binding_point{ ~0u };
			uint32_t subresource{ ~0u };

			gfx::Texture* texture{ nullptr };
			gfx::Sampler* sampler{ nullptr };

			gfx::Buffer* buffer{ nullptr };
		};

		class UniformHandle
		{
		public:
			UniformHandle(const gfx::ShaderReflection& reflection, const gfx::ShaderResource& resource) :
				reflection{ reflection },
				resource{ resource }
			{
				data.resize(resource.size);
			}

			// For render script
			void Set(uint32_t name, const void* raw_data, u64 size)
			{
				auto var_begin = reflection.shader_variables.Begin() + resource.range_begin;
				auto var_end = reflection.shader_variables.Begin() + resource.variable_count;

				auto found = std::find_if(var_begin, var_end, [&name](const gfx::ShaderVariable& variable) { return variable.name_hash == name; });
				if (found == var_end)
				{
					assert(false && "Uniform variable is not found.");
					return;
				}
			
				auto& variable = *found;
				if (size == 0ull)
					size = variable.size;
			
				std::memcpy(data.data() + variable.offset, raw_data, size);
			}
			
			template<class _Ty>
			void Set(uint32_t name, const _Ty& object, u64 size = 0ull)
			{
				auto var_begin = reflection.shader_variables.Begin() + resource.range_begin;
				auto var_end = reflection.shader_variables.Begin() + resource.variable_count;

				auto found = std::find_if(var_begin, var_end, [&name](const gfx::ShaderVariable& variable) { return variable.name_hash == name; });
				if (found == var_end)
				{
					assert(false && "Uniform variable is not found.");
					return;
				}
			
				auto& variable = *found;
				if (size == 0ull)
					size = (std::min)(sizeof(_Ty), variable.size);
			
				std::memcpy(data.data() + variable.offset, &object, size);
			}
			
			template<class _Ty>
			_Ty* Get(uint32_t name)
			{
				auto var_begin = reflection.shader_variables.Begin() + resource.range_begin;
				auto var_end = reflection.shader_variables.Begin() + resource.variable_count;

				auto found = std::find_if(var_begin, var_end, [&name](const gfx::ShaderVariable& variable) { return variable.name_hash == name; });
				if (found == var_end)
				{
					assert(false && "Uniform variable is not found.");
					return nullptr;
				}
			
				auto& variable = *found;
			
				if (sizeof(_Ty) > variable.size)
					return nullptr;
			
				return reinterpret_cast<_Ty*>(data.data() + variable.offset);
			}
			
			const std::vector<char>& Data() const
			{
				return data;
			}

			uint32_t GetNameHash() const
			{
				return resource.name_hash;
			}

			const gfx::ShaderResource& GetResource()
			{
				return resource;
			}

		private:
			const gfx::ShaderReflection& reflection;
			const gfx::ShaderResource& resource;
			utl::vector<char> data{};
		};

		class ShaderPass
		{
		public:
			ShaderPass(gfx::Context& context, const PassCreateInfo& create_info);
			virtual ~ShaderPass();

			void SetExecutionContext(ExecutionContext* exec_context);

			void Execute(float delta_time);

			UniformHandle& GetUniformBlock(uint32_t name_hash);
			UniformHandle& GetUniformBlock(const char* name);

			UniformHandle GetConstantBlock();

			void BindResource(const ShaderResourceBinding& resource_binding);

			virtual ShaderPassType GetPassType() = 0;

			bool CanExecute() const;
		protected:
			void FlushResources(gfx::CommandList& command_list);
			void PushConstantRanges(gfx::CommandList& command_list);
			void CollectProfileData();
			virtual void _Execute(gfx::CommandList& command_list, float delta_time) = 0;
		protected:
			gfx::Context& context;

			std::string debug_name{};
			bool one_time{ false };
			bool can_execute{ true };

			gfx::SharedDescriptor* shared_descriptor{ nullptr };
			gfx::ResourceDescriptor* resource_descriptor{ nullptr };
			ExecutionContext* execution_context{ nullptr };

			std::vector<UniformHandle> uniform_handles;
			std::unique_ptr<UniformHandle> constant_block;

			gfx::Shader* shader{ nullptr };
			gfx::RootSignature* root_signature{ nullptr };
			gfx::Pipeline* pipeline{ nullptr };

			gfx::QueryPool* timestamp_query{ nullptr };
			uint64_t execution_time{ 0ull };
			float timestamp_frequency{ 0.f };
			float pass_delta_time{ 0.f };
		};
	}
}