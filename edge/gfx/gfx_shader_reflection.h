#pragma once

#include "gfx_enum.h"
#include "../util/utl_hash.hpp"
#include "../util/utl_vector.h"

namespace edge::gfx {
	struct ReflectionBase {
		static constexpr size_t k_max_name_len{ 128 };

		void set_name(const char* n) {
			strncpy(name, n, strnlen(n, k_max_name_len));
			name_hash = murmur_hash(n);
		}

		char name[k_max_name_len];
		uint32_t name_hash{};
	};


	// Shader structures
	struct VertexInput : ReflectionBase {
		uint32_t location{};

		uint32_t vec_size{};
		uint32_t columns{};
		uint32_t size{};
	};

	struct ShaderResource : ReflectionBase {
		// Resource descriptor type
		DescriptorType type{};

		// Set where resource is located
		uint32_t set{};

		// Resource binding inside set
		uint32_t binding{};

		// Shader stages where resource was used
		ShaderStageFlags stages{};

		// Resource whole size
		uint32_t size{};

		uint32_t count{ 1u };

		// Range of variable infos
		uint32_t range_begin{ ~0u };
		uint32_t variable_count{};
	};

	struct ShaderVariable : ReflectionBase {
		uint32_t offset{};
		uint32_t size{};
	};

	struct ShaderReflection {
		auto lookup_shader_resource(const char* n) -> std::tuple<bool, ShaderResource&> {
			uint32_t name_hash = murmur_hash(n);
			auto found =  std::find_if(shader_resources.begin(), shader_resources.end(),
				[&name_hash](const ShaderResource& resource) {
					return resource.name_hash == name_hash;
				});

			if (found != shader_resources.end()) {
				return { true, *found };
			}

			auto& new_resource = add_shader_resource();
			strncpy(new_resource.name, n, strnlen(n, ShaderResource::k_max_name_len));
			new_resource.name_hash = name_hash;

			return { false, new_resource };
		}

		utl::fixed_vector<VertexInput, 16ull> vertex_inputs{};

		VertexInput& add_vertex_input() {
			return vertex_inputs.emplace_back();
		}

		// Uniforms/storage blocks/textures/samplers etc.
		utl::fixed_vector<ShaderResource, 32ull> shader_resources{};

		ShaderResource& add_shader_resource() {
			return shader_resources.emplace_back();
		}

		// Variables inside uniforms/push constants
		utl::fixed_vector<ShaderVariable, 64ull> shader_variables{};

		ShaderVariable& add_shader_variable(const char* n = nullptr) {
			auto& new_var = shader_variables.emplace_back();
			if (n) {
				new_var.set_name(n);
			}

			return new_var;
		}

		ShaderResource constants{};

		uint32_t invocation_count{};

		uint32_t output_vertex_count{};

		// Thread group sizes
		std::array<uint32_t, 3ull> local_sizes{};
	};
}