#pragma once

#include "gfx_enum.h"

namespace edge
{
	namespace gfx
	{
		template<typename _Ty, size_t element_count>
		struct StaticArray
		{
			std::array<_Ty, element_count> data{};
			uint32_t count{};

			_Ty& NewElement()
			{
				assert(count + 1u < element_count);
				return data[count++];
			}

			auto Begin()
			{
				return data.begin();
			}

			auto Begin() const
			{
				return data.begin();
			}

			auto End()
			{
				assert(count < element_count);
				return data.begin() + count;
			}

			auto End() const
			{
				assert(count < element_count);
				return data.begin() + count;
			}
		};


		// Shader structures
		struct VertexInput
		{
			char name[255];

			uint32_t name_hash{};

			uint32_t location{};

			uint32_t vec_size{};
			uint32_t columns{};
			uint32_t size{};
		};

		struct ShaderResource
		{
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

			// Name string
			char name[255];

			// Name hash
			uint32_t name_hash{};

			// Range of variable infos
			uint32_t range_begin{ ~0u };
			uint32_t variable_count{};
		};

		struct ShaderVariable
		{
			// Variable name
			char name[255];

			// Variable name hash
			uint32_t name_hash{};

			uint32_t offset{};

			uint32_t size{};
		};

		struct ShaderReflection
		{
			StaticArray<VertexInput, 16ull> vertex_inputs{};

			VertexInput& AddVertexInput()
			{
				return vertex_inputs.NewElement();
			}

			// Uniforms/storage blocks/textures/samplers etc.
			StaticArray<ShaderResource, 64ull> shader_resources{};

			ShaderResource& AddShaderResource()
			{
				return shader_resources.NewElement();
			}

			// Variables inside uniforms/push constants
			StaticArray<ShaderVariable, 128ull> shader_variables{};

			ShaderVariable& AddShaderVariable()
			{
				return shader_variables.NewElement();
			}

			ShaderResource constants{};

			uint32_t invocation_count{};

			uint32_t output_vertex_count{};

			// Thread group sizes
			std::array<float, 3ull> local_sizes{};

		};
	}
}