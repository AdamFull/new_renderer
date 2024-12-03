#pragma once

#include "gfx_structures.h"

namespace engine
{
	namespace gfx
	{
		// Shader effect compiler stuff
		namespace shfx
		{
			struct ShaderEffectCompileSourceDesc
			{
				std::string source;
				ShaderStageFlag type;
				std::vector<std::string> definitions{};
			};

			void to_json(nlohmann::json& json, const ShaderEffectCompileSourceDesc& type);
			void from_json(const nlohmann::json& json, ShaderEffectCompileSourceDesc& type);

			struct ShaderEffectCompileSamplerDesc
			{
				std::string binding;
				SamplerCreateInfo sampler_create_info;
			};

			void to_json(nlohmann::json& json, const ShaderEffectCompileSamplerDesc& type);
			void from_json(const nlohmann::json& json, ShaderEffectCompileSamplerDesc& type);

			struct ShaderEffectCompilePipelineState
			{
				RasterizationState rasterization_state{};
				MultisampleState multisample_state{};
				DepthStencilState depth_stencil_state{};
				ColorBlendingState color_blending_state{};

				PrimitiveTopology primitive_topology{ PrimitiveTopology::eTriangleList };
			};

			void to_json(nlohmann::json& json, const ShaderEffectCompilePipelineState& type);
			void from_json(const nlohmann::json& json, ShaderEffectCompilePipelineState& type);

			struct ShaderEffectCompileDesc
			{
				std::string name;
				std::vector<ShaderEffectCompileSourceDesc> shaders{};
				std::vector<ShaderEffectCompileSamplerDesc> samplers{};
				ShaderEffectCompilePipelineState pipeline_state{};
			};

			void to_json(nlohmann::json& json, const ShaderEffectCompileDesc& type);
			void from_json(const nlohmann::json& json, ShaderEffectCompileDesc& type);

			struct ShaderEffectDesc
			{
				ShaderCreateInfo shader_create_info{};
				std::vector<SamplerCreateInfo> sampler_create_infos{};
				PipelineType pipeline_type{};

				RasterizationState rasterization_state{};
				MultisampleState multisample_state{};
				DepthStencilState depth_stencil_state{};
				ColorBlendingState color_blending_state{};

				PrimitiveTopology primitive_topology{};
			};

			void to_json(nlohmann::json& json, const ShaderEffectDesc& type);
			void from_json(const nlohmann::json& json, ShaderEffectDesc& type);

			// THis one will be saved after compillation
			struct ShaderEffectCreateInfo
			{
				ShaderEffectDesc desc{};
				PipelineCache* pipeline_cache{ nullptr };

				std::array<Format, k_max_attachment_count> color_formats{};
				uint32_t color_format_count{};

				Format depth_format{ Format::eUndefined };
				Format stencil_format{ Format::eUndefined };
			};

			// Contains final shader data
			struct ShaderEffect
			{
				Shader* shader{ nullptr };
				RootSignature* root_signature{ nullptr };
				Pipeline* pipeline{ nullptr };
				std::unordered_map<uint32_t, Sampler*> samplers{};
			};
		}
	}
}