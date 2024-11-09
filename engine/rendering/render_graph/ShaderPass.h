#pragma once

#include "RenderGraphInterface.h"
#include <engine/rendering/ExecutionContext.h>
#include <engine/rendering/gfx/Base.h>

#include <algorithm>

#ifdef MemoryBarrier
#undef MemoryBarrier
#endif

namespace engine
{
	namespace gfx
	{
		class Context;
	}

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
			gfx::Shader* shader{ nullptr };
			gfx::PipelineCache* pipeline_cache{ nullptr };
			gfx::SharedDescriptor* shared_descriptor{ nullptr };

			std::function<void(ShaderPass&, gfx::CommandList&, float)> draw_callback{};

			std::string debug_name{};
			bool one_time{ false };
		};

		struct ComputePassCreateInfo : PassCreateInfo
		{
			
		};

		struct RenderPassCreateInfo : PassCreateInfo
		{
			gfx::PipelineState pipeline_state{};
			std::vector<RenderResourceAttachment> resource_attachments{};
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
					size = std::min(sizeof(_Ty), static_cast<size_t>(variable.size));
			
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

		class ShaderPass : public IGraphNode
		{
		public:
			ShaderPass(IRenderGraph& graph, const PassCreateInfo& create_info);
			virtual ~ShaderPass();

			void SetExecutionContext(ExecutionContext* exec_context);

			void Evaluate(gfx::CommandList& command_list, float delta_time) override;

			UniformHandle& GetUniformBlock(uint32_t name_hash);
			UniformHandle& GetUniformBlock(const char* name);

			UniformHandle& GetConstantBlock();

			void ReadResource(uint32_t binding_point, uint32_t render_resource_id, uint32_t subresource_index = ~0u);
			void WriteResource(uint32_t binding_point, uint32_t render_resource_id, uint32_t subresource_index = ~0u);

			void ResolveDependencies(gfx::CommandList& command_list);

			void BindResource(uint32_t binding_point, uint32_t resource_index, uint32_t subresource_index = ~0u);
			void BindReadResource(uint32_t binding_point, uint32_t resource_index, uint32_t subresource_index = ~0u);
			void BindWriteResource(uint32_t binding_point, uint32_t resource_index, uint32_t subresource_index = ~0u);

			virtual void Begin(gfx::CommandList& command_list) = 0;
			virtual void End(gfx::CommandList& command_list) = 0;

			virtual ShaderPassType GetPassType() = 0;

			bool CanExecute() const;
			void BindResources(gfx::CommandList& command_list);
			void PushConstantRanges(gfx::CommandList& command_list);
			void DivideWorkgroupByLocalSizes(uint32_t& group_x, uint32_t& group_y, uint32_t& group_z);

		protected:
			void ResourceDependency(uint32_t binding_point, uint32_t resource_id, uint32_t sebresource_id, gfx::AccessTypeFlags access);
			void CollectProfileData();
			virtual void _Execute(gfx::CommandList& command_list, float delta_time) = 0;
		protected:
			IRenderGraph& graph;
			gfx::Context& context;

			std::string debug_name{};
			bool one_time{ false };
			bool can_execute{ true };

			std::function<void(ShaderPass&, gfx::CommandList&, float)> draw_callback{};

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

			// Per frame barriers

			std::array<gfx::BufferBarrier, 64ull> buffer_barriers{};
			std::array<gfx::TextureBarrier, 64ull> texture_barriers{};
			std::array<gfx::MemoryBarrier, 64ull> memory_barriers{};
			gfx::ResourceBarrierInfo resource_barriers{};
		};
	}
}