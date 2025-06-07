#pragma once

#include "../ExecutionContext.h"

#include <algorithm>

namespace engine::render {
	class IRenderer;
	class ShaderPass;

	enum class ShaderPassType {
		eNone,
		eCompute,
		eGraphics
	};

	struct PassCreateInfo {
		std::string_view name;
		edge::gfx::IShader* shader{ nullptr };
		edge::gfx::IRootSignature* root_signature{ nullptr };
		edge::gfx::IPipeline* pipeline{ nullptr };
		edge::utl::fixed_vector<edge::gfx::IDescriptorLayout*> descriptor_layouts;

		std::function<void(ShaderPass&, edge::gfx::ICommandList&, float)> draw_callback{};
		std::function<bool()> enabled_callback{};

		bool one_time{ false };
	};

	struct ComputePassCreateInfo : PassCreateInfo {
	};

	struct RenderPassCreateInfo : PassCreateInfo {
		std::vector<RenderResourceAttachment> resource_attachments{};
	};

	struct ShaderResourceBinding {
		uint32_t binding_point{ ~0u };
		uint32_t subresource{ ~0u };

		edge::gfx::ITexture* texture{ nullptr };
		edge::gfx::ISampler* sampler{ nullptr };

		edge::gfx::IBuffer* buffer{ nullptr };
	};

	class UniformHandle {
	public:
		UniformHandle(const edge::gfx::ShaderReflection& reflection, const edge::gfx::ShaderResource& resource) :
			reflection{ reflection },
			resource{ resource } {
			data_.resize(resource.size);
		}

		// For render script
		void set(uint32_t name, const void* raw_data, uint64_t size) {
			auto var_begin = reflection.shader_variables.begin() + resource.range_begin;
			auto var_end = reflection.shader_variables.begin() + resource.variable_count;

			auto found = std::find_if(var_begin, var_end, 
				[&name](const edge::gfx::ShaderVariable& variable) { return variable.name_hash == name; });

			if (found == var_end) {
				assert(false && "Uniform variable is not found.");
				return;
			}

			auto& variable = *found;
			if (size == 0ull) {
				size = variable.size;
			}

			std::memcpy(data_.data() + variable.offset, raw_data, size);
		}

		template<class _Ty>
		void set(uint32_t name, const _Ty& object, uint64_t size = 0ull) {
			auto var_begin = reflection.shader_variables.begin() + resource.range_begin;
			auto var_end = reflection.shader_variables.begin() + resource.variable_count;

			auto found = std::find_if(var_begin, var_end,
				[&name](const edge::gfx::ShaderVariable& variable) { return variable.name_hash == name; });

			if (found == var_end) {
				assert(false && "Uniform variable is not found.");
				return;
			}

			auto& variable = *found;
			if (size == 0ull) {
				size = std::min(sizeof(_Ty), static_cast<size_t>(variable.size));
			}

			std::memcpy(data_.data() + variable.offset, &object, size);
		}

		template<class _Ty>
		_Ty* get(uint32_t name) {
			auto var_begin = reflection.shader_variables.begin() + resource.range_begin;
			auto var_end = reflection.shader_variables.begin() + resource.variable_count;

			auto found = std::find_if(var_begin, var_end,
				[&name](const edge::gfx::ShaderVariable& variable) { return variable.name_hash == name; });

			if (found == var_end) {
				assert(false && "Uniform variable is not found.");
				return nullptr;
			}

			auto& variable = *found;

			if (sizeof(_Ty) > variable.size) {
				return nullptr;
			}

			return reinterpret_cast<_Ty*>(data_.data() + variable.offset);
		}

		template<typename _Ty>
		_Ty& as(size_t offset = 0ull) {
			assert(sizeof(_Ty) + offset <= resource.size);
			return *reinterpret_cast<_Ty*>(&data_[offset]);
		}

		const std::vector<char>& data() const {
			return data_;
		}

		uint32_t get_name_hash() const {
			return resource.name_hash;
		}

		const edge::gfx::ShaderResource& get_resource() {
			return resource;
		}

	private:
		const edge::gfx::ShaderReflection& reflection;
		const edge::gfx::ShaderResource& resource;
		std::vector<char> data_{};
	};

	class ShaderPass : public IExecutionNode {
	public:
		ShaderPass(IExecutionContext& execution_context, const PassCreateInfo& create_info);
		virtual ~ShaderPass();

		void execute(edge::gfx::ICommandList& command_list, float delta_time) override;

		auto get_uniform_block(uint32_t name_hash) -> UniformHandle&;
		auto get_uniform_block(const char* name) -> UniformHandle&;

		auto get_constant_block() -> UniformHandle&;

		void read_resource(uint32_t binding_point, uint32_t render_resource_id, uint32_t subresource_index);
		void read_resource(uint32_t render_resource_id, uint32_t subresource_index);
		void write_resource(uint32_t binding_point, uint32_t render_resource_id, uint32_t subresource_index);
		void write_resource(uint32_t render_resource_id, uint32_t subresource_index);

		void resolve_dependencies(edge::gfx::ICommandList& command_list);

		void bind_resource(uint32_t binding_point, uint32_t resource_index, uint32_t subresource_index = 0u);
		void bind_read_resource(uint32_t binding_point, uint32_t resource_index, uint32_t subresource_index = 0u);
		void bind_write_resource(uint32_t binding_point, uint32_t resource_index, uint32_t subresource_index);

		void dispatch_for_buffer(edge::gfx::ICommandList& command_list, uint32_t buffer_id, bool divide_by_locals);
		void dispatch_for_target(edge::gfx::ICommandList& command_list, uint32_t target_it, bool divide_by_locals);

		virtual void begin(edge::gfx::ICommandList& command_list) = 0;
		virtual void end(edge::gfx::ICommandList& command_list) = 0;

		virtual ShaderPassType get_pass_type() = 0;

		bool can_execute() const;
		void bind_resources(edge::gfx::ICommandList& command_list);
		void push_constant_ranges(edge::gfx::ICommandList& command_list);
		void divide_workgroup_by_local_sizes(uint32_t& group_x, uint32_t& group_y, uint32_t& group_z);

	protected:
		void resource_dependency(uint32_t resource_id, uint32_t sebresource_id, edge::gfx::AccessTypeFlags access);
		void resource_dependency(uint32_t binding_point, uint32_t resource_id, uint32_t sebresource_id, edge::gfx::AccessTypeFlags access);
		virtual void _execute(edge::gfx::ICommandList& command_list, float delta_time) = 0;
	protected:
		IExecutionContext& execution_context;

		std::string debug_name{};
		bool one_time{ false };
		bool can_execute_{ true };

		std::function<void(ShaderPass&, edge::gfx::ICommandList&, float)> draw_callback{};
		std::function<bool()> enabled_callback{};

		std::vector<UniformHandle> uniform_handles;
		std::unique_ptr<UniformHandle> constant_block;

		edge::gfx::IShader* shader{ nullptr };
		edge::gfx::IRootSignature* root_signature{ nullptr };
		edge::gfx::IPipeline* pipeline{ nullptr };
		edge::utl::fixed_vector<edge::gfx::IDescriptorLayout*> descriptor_layouts;

		// Per frame barriers
		std::vector<edge::gfx::BufferBarrierDesc> buffer_barriers{};
		std::vector<edge::gfx::TextureBarrierDesc> texture_barriers{};
		std::vector<edge::gfx::MemoryBarrierDesc> memory_barriers{};
		edge::gfx::ResourceBarrierInfo resource_barriers{};
	};
}