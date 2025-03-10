#include "gfx_technique_system.h"

#include "shader_compiler/glsl/gfx_glsl_shader_compiler.h"

#include "../gfx_context.h"

#include "../../util/fs_helper.h"
#include "../../util/Logger.h"
#include "../../filesystem/VirtualFileSystemInterface.h"

namespace edge
{
	namespace gfx
	{
		constexpr const std::string_view k_frame_graph_cache_path = "/root/fgcache";

		TechniqueDatabase::TechniqueDatabase(IContext* gfx_context, filesystem::IVirtualFileSystemInterface* filesystem) :
			gfx_context{ gfx_context },
			filesystem{ filesystem }
		{
			ShaderCompilerCreateInfo compiler_ci{};
			compiler_ci.filesystem = filesystem;
			//TODO: get this data from database ci
			compiler_ci.base_path = "/source/embed/shaders";
			compiler_ci.additional_include_dirs = { "includes" };

			shader_compiler = std::make_unique<GLSLShaderCompiler>(compiler_ci);

			std::vector<uint8_t> pipeline_cache_data{};
			if (filesystem->Exists(k_frame_graph_cache_path))
			{
				filesystem->ReadFile(k_frame_graph_cache_path, pipeline_cache_data);
			}

			PipelineCacheCreateInfo pipeline_cache_ci{};
			pipeline_cache_ci.data = pipeline_cache_data;

			pipeline_cache = gfx_context->CreatePipelineCache(pipeline_cache_ci);
		}

		TechniqueDatabase::~TechniqueDatabase()
		{
			for (auto& [name, technique] : techniques)
			{
				gfx_context->Destroy(technique.pipeline);
				gfx_context->Destroy(technique.root_signature);
				gfx_context->Destroy(technique.shader);
			}

			techniques.clear();

			size_t cache_size{ 0 };
			pipeline_cache->GetData(cache_size, nullptr);

			if (cache_size)
			{
				std::vector<uint8_t> data(cache_size, '\0');
				pipeline_cache->GetData(cache_size, data.data());

				filesystem->WriteFile(k_frame_graph_cache_path, data);
			}

			gfx_context->Destroy(pipeline_cache);
			pipeline_cache = nullptr;
		}

		void TechniqueDatabase::AddTechnique(std::string_view name, const TechniqueDesc& technique_desc)
		{
			std::vector<ShaderStageInfo> shader_stages{};

			if (technique_desc.type == TechniqueType::eNone)
			{
				EDGE_LOGE("Incorrect technique type for technique named: {}.", name);
				return;
			}

			for (auto& stage : technique_desc.stages)
			{
				ShaderStageFlag stage_flag{};

				auto ext = fsh::get_ext(stage.path);
				if (ext.compare(".vert") == 0)
					stage_flag = ShaderStageFlag::eVertex;
				else if (ext.compare(".tesc") == 0)
					stage_flag = ShaderStageFlag::eTessellationControl;
				else if (ext.compare(".tese") == 0)
					stage_flag = ShaderStageFlag::eTessellationEvaluation;
				else if (ext.compare(".geom") == 0)
					stage_flag = ShaderStageFlag::eGeometry;
				else if (ext.compare(".frag") == 0)
					stage_flag = ShaderStageFlag::eFragment;
				else if (ext.compare(".comp") == 0)
					stage_flag = ShaderStageFlag::eCompute;

				shader_stages.push_back({
					.stage = stage_flag,
					.entry_point = "main",
					.code = GetOrCompileShader(stage)
					});
			}

			IShader* shader = gfx_context->CreateShader(shader_stages);

			RootSignatureCreateInfo root_sign_ci{};
			root_sign_ci.shader = shader;
			root_sign_ci.shared_descriptor = technique_desc.shared_descriptor;
			IRootSignature* root_signature = gfx_context->CreateRootSignature(root_sign_ci);

			IPipeline* pipeline{ nullptr };
			if (technique_desc.type == TechniqueType::eCompute)
			{
				ComputePipelineCreateInfo pipeline_ci{};
				pipeline_ci.shader = shader;
				pipeline_ci.root_signature = root_signature;
				pipeline_ci.pipeline_cache = pipeline_cache;
				pipeline = gfx_context->CreatePipeline(pipeline_ci);
			}
			else if (technique_desc.type == TechniqueType::eGraphics)
			{
				GraphicsPipelineCreateInfo pipeline_ci{};
				pipeline_ci.shader = shader;
				pipeline_ci.root_signature = root_signature;
				pipeline_ci.pipeline_cache = pipeline_cache;

				for (const auto& attribute : technique_desc.vertex_layout.attributes)
				{
					pipeline_ci.pipeline_state.vertex_layout.attributes[pipeline_ci.pipeline_state.vertex_layout.attribute_count++] = attribute;
				}

				for (const auto& binding : technique_desc.vertex_layout.bindings)
				{
					pipeline_ci.pipeline_state.vertex_layout.bindings[pipeline_ci.pipeline_state.vertex_layout.binding_count++] = binding;
				}

				for (const auto& attachment : technique_desc.color_blending_attachments)
				{
					pipeline_ci.pipeline_state.color_blending_state.attachments[pipeline_ci.pipeline_state.color_blending_state.attachment_count++] = attachment;
				}

				pipeline_ci.pipeline_state.rasterization_state = technique_desc.rasterization_state;
				pipeline_ci.pipeline_state.multisample_state = technique_desc.multisample_state;
				pipeline_ci.pipeline_state.depth_stencil_state = technique_desc.depth_stencil_state;
				pipeline_ci.pipeline_state.primitive_topology = technique_desc.primitive_topology;

				for (Format format : technique_desc.color_formats)
				{
					pipeline_ci.color_formats[pipeline_ci.color_format_count++] = format;
				}

				pipeline_ci.depth_format = technique_desc.depth_format;
				pipeline_ci.stencil_format = technique_desc.stencil_format;
				pipeline = gfx_context->CreatePipeline(pipeline_ci);
			}

			techniques.emplace(name, Technique{ name, shader, root_signature, pipeline });
		}

		auto TechniqueDatabase::GetTechnique(std::string_view name) -> Technique&
		{
			static Technique fallback_technique{};

			auto found = techniques.find(name);
			if (found != techniques.end())
			{
				return found->second;
			}

			return fallback_technique;
		}

		std::span<const uint8_t> TechniqueDatabase::GetOrCompileShader(const TechniqueDesc::ShaderStage& stage)
		{
			ShaderCompileInfo shader_compile_info{};
			shader_compile_info.path = stage.path;
			shader_compile_info.definitions = stage.definitions;

			// This hash key needed to keep some cache for compiled shaders
			uint32_t unique_shader_key{ 0u };
			unique_shader_key ^= murmur_hash(stage.path);

			for(const auto& define : stage.definitions)
				unique_shader_key ^= murmur_hash(define);

			// Return found shader code
			auto found_code = shader_code.find(unique_shader_key);
			if (found_code != shader_code.end())
				return found_code->second;

			EDGE_LOGI("Compiling shader: {}.", stage.path);

			std::string shader_source{};
			if (filesystem->Exists(stage.path) && filesystem->ReadFile(stage.path, shader_source))
			{
				shader_compile_info.source = shader_source;

				std::vector<uint8_t> compiled_code{};
				if (shader_compiler->Compile(shader_compile_info, compiled_code))
				{
					EDGE_LOGI("Compiled shader: {}.", stage.path);
					shader_code.emplace(unique_shader_key, std::move(compiled_code));
					return shader_code[unique_shader_key];
				}
			}

			EDGE_LOGE("Failed to compile shader: {}.", stage.path);

			return {};
		}
	}
}