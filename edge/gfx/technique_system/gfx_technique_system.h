#pragma once
#include "../../util/utl_hash.hpp"
#include "../gfx_structures.h"

#include "gfx_shader_compiler.h"

#include <deque>

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
				std::vector<std::string> definitions;
			};

			TechniqueType type{};
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
			std::string name;
			IShader* shader{ nullptr };
			IRootSignature* root_signature{ nullptr };
			IPipeline* pipeline{ nullptr };
			std::vector<IDescriptorLayout*> descriptor_layouts;
		};

		struct Shader {
			std::vector<uint8_t> code;
			uint32_t shader_hash{ 0u };
			uint32_t dependency_hash{ 0u };
		};

		struct TechniqueConfigInfo {
			std::string_view db_path;
			Format backbuffer_format{};
		};

		TechniqueDatabase();
		~TechniqueDatabase();

		bool load_config(const TechniqueConfigInfo& config);

		bool load(const std::string& path);
		bool save(const std::string& path);

		void add_descriptor_layout(IDescriptorLayout* layout);

		void add_technique(const std::string& name, const TechniqueDesc& technique_desc);
		auto get_technique(const std::string& name) -> Technique&;
	private:
		auto create_descriptor_table(IShader* shader, std::vector<IDescriptorLayout*>& table_layouts) -> void;

		std::span<const uint8_t> get_or_compile_shader(const TechniqueDesc::ShaderStage& stage);

		IPipelineCache* pipeline_cache{ nullptr };

		std::unique_ptr<IShaderCompiler> shader_compiler;

		std::unordered_map<uint32_t, Shader> shaders;
		std::unordered_map<std::string, Technique> techniques;

		std::deque<IDescriptorLayout*> descriptor_layouts;
	};
}