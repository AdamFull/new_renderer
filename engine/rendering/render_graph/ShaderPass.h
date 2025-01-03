#pragma once

#include "../ExecutionContext.h"

#include <algorithm>

#ifdef MemoryBarrier
#undef MemoryBarrier
#endif

namespace engine
{
	namespace render
	{
		class IRenderer;
		class ShaderPass;

		enum class ShaderPassType
		{
			eNone,
			eCompute,
			eGraphics
		};

		struct PassCreateInfo
		{
			edge::gfx::ShaderEffect* shader{ nullptr };
			edge::gfx::PipelineCache* pipeline_cache{ nullptr };
			edge::gfx::SharedDescriptor* shared_descriptor{ nullptr };

			std::function<void(ShaderPass&, edge::gfx::CommandList&, float)> draw_callback{};
			std::function<bool()> enabled_callback{};

			std::string debug_name{};
			bool one_time{ false };
		};

		struct ComputePassCreateInfo : PassCreateInfo
		{
			
		};

		struct RenderPassCreateInfo : PassCreateInfo
		{
			edge::gfx::PipelineState pipeline_state{};
			std::vector<RenderResourceAttachment> resource_attachments{};
		};

		struct ShaderResourceBinding
		{
			uint32_t binding_point{ ~0u };
			uint32_t subresource{ ~0u };

			edge::gfx::Texture* texture{ nullptr };
			edge::gfx::Sampler* sampler{ nullptr };

			edge::gfx::Buffer* buffer{ nullptr };
		};

		class UniformHandle
		{
		public:
			UniformHandle(const edge::gfx::ShaderReflection& reflection, const edge::gfx::ShaderResource& resource) :
				reflection{ reflection },
				resource{ resource }
			{
				data.resize(resource.size);
			}

			// For render script
			void Set(uint32_t name, const void* raw_data, uint64_t size)
			{
				auto var_begin = reflection.shader_variables.Begin() + resource.range_begin;
				auto var_end = reflection.shader_variables.Begin() + resource.variable_count;

				auto found = std::find_if(var_begin, var_end, [&name](const edge::gfx::ShaderVariable& variable) { return variable.name_hash == name; });
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
			void Set(uint32_t name, const _Ty& object, uint64_t size = 0ull)
			{
				auto var_begin = reflection.shader_variables.Begin() + resource.range_begin;
				auto var_end = reflection.shader_variables.Begin() + resource.variable_count;

				auto found = std::find_if(var_begin, var_end, [&name](const edge::gfx::ShaderVariable& variable) { return variable.name_hash == name; });
				if (found == var_end)
				{
					assert(false && "Uniform variable is not found.");
					return;
				}
			
				auto& variable = *found;
				if (size == 0ull)
					size = std::min(sizeof(_Ty), static_cast<size_t>(variable.size));
			
				std::memcpy(data.data() + variable.offset, &object, size);
			}
			
			template<class _Ty>
			_Ty* Get(uint32_t name)
			{
				auto var_begin = reflection.shader_variables.Begin() + resource.range_begin;
				auto var_end = reflection.shader_variables.Begin() + resource.variable_count;

				auto found = std::find_if(var_begin, var_end, [&name](const edge::gfx::ShaderVariable& variable) { return variable.name_hash == name; });
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

			template<typename _Ty>
			_Ty& As(size_t offset = 0ull)
			{
				assert(sizeof(_Ty) + offset <= resource.size);
				return *reinterpret_cast<_Ty*>(&data[offset]);
			}
			
			const std::vector<char>& Data() const
			{
				return data;
			}

			uint32_t GetNameHash() const
			{
				return resource.name_hash;
			}

			const edge::gfx::ShaderResource& GetResource()
			{
				return resource;
			}

		private:
			const edge::gfx::ShaderReflection& reflection;
			const edge::gfx::ShaderResource& resource;
			std::vector<char> data{};
		};

		class ShaderPass : public ExecutionNode
		{
		public:
			ShaderPass(ExecutionContext& execution_context, const PassCreateInfo& create_info);
			virtual ~ShaderPass();

			void Execute(edge::gfx::CommandList& command_list, float delta_time) override;

			UniformHandle& GetUniformBlock(uint32_t name_hash);
			UniformHandle& GetUniformBlock(const char* name);

			UniformHandle& GetConstantBlock();

			void ReadResource(uint32_t binding_point, uint32_t render_resource_id, uint32_t subresource_index);
			void ReadResource(uint32_t render_resource_id, uint32_t subresource_index);
			void WriteResource(uint32_t binding_point, uint32_t render_resource_id, uint32_t subresource_index);
			void WriteResource(uint32_t render_resource_id, uint32_t subresource_index);

			void ResolveDependencies(edge::gfx::CommandList& command_list);

			void BindResource(uint32_t binding_point, uint32_t resource_index, uint32_t subresource_index);
			void BindReadResource(uint32_t binding_point, uint32_t resource_index, uint32_t subresource_index);
			void BindWriteResource(uint32_t binding_point, uint32_t resource_index, uint32_t subresource_index);

			void DispatchForTarget(edge::gfx::CommandList& command_list, uint32_t target_it, bool divide_by_locals);
			void FinalizeTarget(edge::gfx::CommandList& command_list, uint32_t binding_point, uint32_t target);

			virtual void Begin(edge::gfx::CommandList& command_list) = 0;
			virtual void End(edge::gfx::CommandList& command_list) = 0;

			virtual ShaderPassType GetPassType() = 0;

			bool CanExecute() const;
			void BindResources(edge::gfx::CommandList& command_list);
			void PushConstantRanges(edge::gfx::CommandList& command_list);
			void DivideWorkgroupByLocalSizes(uint32_t& group_x, uint32_t& group_y, uint32_t& group_z);

		protected:
			void ResourceDependency(uint32_t resource_id, uint32_t sebresource_id, edge::gfx::AccessTypeFlags access);
			void ResourceDependency(uint32_t binding_point, uint32_t resource_id, uint32_t sebresource_id, edge::gfx::AccessTypeFlags access);
			void CollectProfileData();
			virtual void _Execute(edge::gfx::CommandList& command_list, float delta_time) = 0;
		protected:
			ExecutionContext& execution_context;
			edge::gfx::Context& context;

			std::string debug_name{};
			bool one_time{ false };
			bool can_execute{ true };

			std::function<void(ShaderPass&, edge::gfx::CommandList&, float)> draw_callback{};
			std::function<bool()> enabled_callback{};

			edge::gfx::SharedDescriptor* shared_descriptor{ nullptr };
			edge::gfx::ResourceDescriptor* resource_descriptor{ nullptr };

			std::vector<UniformHandle> uniform_handles;
			std::unique_ptr<UniformHandle> constant_block;

			edge::gfx::ShaderEffect* shader_effect{ nullptr };
			edge::gfx::RootSignature* root_signature{ nullptr };
			edge::gfx::Pipeline* pipeline{ nullptr };

			edge::gfx::QueryPool* timestamp_query{ nullptr };
			uint64_t execution_time{ 0ull };
			float timestamp_frequency{ 0.f };
			float pass_delta_time{ 0.f };

			// Per frame barriers

			std::array<edge::gfx::BufferBarrier, 64ull> buffer_barriers{};
			std::array<edge::gfx::TextureBarrier, 64ull> texture_barriers{};
			std::array<edge::gfx::MemoryBarrier, 64ull> memory_barriers{};
			edge::gfx::ResourceBarrierInfo resource_barriers{};
		};
	}
}