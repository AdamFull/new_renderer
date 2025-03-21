#pragma once
#include "../../util/utl_hash.hpp"
#include "gfx_structures.h"

#include "shader_compiler/gfx_shader_compiler.h"

namespace edge::gfx
{
	enum class TechniqueType {
		eNone,
		eCompute,
		eGraphics
	};

	class TechniqueDatabase {
	public:
		struct ShaderDesc {
			std::string_view path;
			std::vector<std::string_view> defines;
			std::vector<std::string_view> permutations;
		};

		struct TechniqueDesc {
			static constexpr size_t k_max_render_target_count = 8;

			struct ShaderStage {
				std::string_view path;
				std::vector<std::string_view> definitions;
			};

			TechniqueType type{};
			ISharedDescriptor* shared_descriptor{ nullptr };
			std::vector<ShaderStage> stages{};

			struct {
				std::vector<VertexAttributeDescription> attributes;
				std::vector<VertexBindingDescription> bindings;
			} vertex_layout;

			std::vector<ColorBlendAttachment> color_blending_attachments;

			RasterizationState rasterization_state{};
			MultisampleState multisample_state{};
			DepthStencilState depth_stencil_state{};

			PrimitiveTopology primitive_topology{ PrimitiveTopology::ePointList };

			std::vector<Format> color_formats{};

			Format depth_format{};
			Format stencil_format{};
		};

		struct Technique {
			std::string_view name;
			IShader* shader{ nullptr };
			IRootSignature* root_signature{ nullptr };
			IPipeline* pipeline{ nullptr };
		};

		struct Shader {
			std::vector<uint8_t> code;
			uint32_t shader_hash{ 0u };
			uint32_t dependency_hash{ 0u };
		};

		TechniqueDatabase();
		~TechniqueDatabase();

		bool load(std::string_view path);
		bool save(std::string_view path);

		void add_technique(std::string_view name, const TechniqueDesc& technique_desc);
		auto get_technique(std::string_view name) -> Technique&;
	private:
		std::span<const uint8_t> get_or_compile_shader(const TechniqueDesc::ShaderStage& stage);

		IPipelineCache* pipeline_cache{ nullptr };

		std::unique_ptr<IShaderCompiler> shader_compiler;

		std::unordered_map<uint32_t, Shader> shaders;
		std::unordered_map<std::string_view, Technique> techniques;
	};
}