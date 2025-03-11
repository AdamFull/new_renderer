#include "gfx_technique_system.h"

#include "shader_compiler/glsl/gfx_glsl_shader_compiler.h"

#include "../gfx_context.h"

#include "../../util/fs_helper.h"
#include "../../util/Logger.h"
#include "../../filesystem/VirtualFileSystemInterface.h"

// TODO: define it from specific config
#define DEVELOPMENT

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

		class BinaryReader
		{
		public:
			BinaryReader(const std::vector<uint8_t>& d) :
				data{ d }
			{
			}

			template<typename _Ty>
			void Read(_Ty& t)
			{
				t = *((_Ty*)(data.data() + pos));
				pos += sizeof(_Ty);
			}

			template<>
			void Read(std::vector<uint8_t>& t)
			{
				auto* ptr = data.data() + pos;
				memcpy(t.data(), ptr, t.size());
				pos += t.size();
			}
		private:
			std::span<const uint8_t> data;
			size_t pos{ 0 };
		};

		class BinaryWriter
		{
		public:
			BinaryWriter(size_t size)
			{
				data.reserve(size);
			}

			template<typename _Ty>
			void Write(const _Ty& t)
			{
				const uint8_t* begin = (const uint8_t*)(&t);
				const uint8_t* end = begin + sizeof(_Ty);
				data.insert(data.end(), begin, end);
			}

			template<>
			void Write(const std::vector<uint8_t>& t)
			{
				data.insert(data.end(), t.begin(), t.end());
			}

			const std::vector<uint8_t>& GetData()
			{
				return data;
			}
		private:
			std::vector<uint8_t> data;
		};

		bool TechniqueDatabase::Load(std::string_view path)
		{
			std::vector<uint8_t> data;
			if (!filesystem->Exists(path) || !filesystem->ReadFile(path, data))
				return false;

			BinaryReader reader(data);

			size_t shader_count;
			reader.Read(shader_count);

			for (int32_t i = 0; i < shader_count; ++i)
			{
				uint32_t hash;
				reader.Read(hash);

				auto& shader = shaders[hash];

				size_t code_size;
				reader.Read(code_size);

				shader.code.resize(code_size);
				reader.Read(shader.code);
				reader.Read(shader.shader_hash);
				reader.Read(shader.dependency_hash);
			}

			return true;
		}

		bool TechniqueDatabase::Save(std::string_view path)
		{
			BinaryWriter writer(8 * 1024 * 1024);

			size_t shader_count = shaders.size();

			// Write shader_count first
			writer.Write(shader_count);

			for (const auto& [hash, shader] : shaders)
			{
				writer.Write(hash);

				writer.Write(shader.code.size());
				writer.Write(shader.code);
				writer.Write(shader.shader_hash);
				writer.Write(shader.dependency_hash);
			}

			return filesystem->WriteFile(path, writer.GetData());
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
			// This hash key needed to keep some cache for compiled shaders
			uint32_t unique_shader_key{ 0u };
			unique_shader_key ^= murmur_hash(stage.path);

			for(const auto& define : stage.definitions)
				unique_shader_key ^= murmur_hash(define);

#if defined(DEVELOPMENT) || defined(EMBEDDED_SHADER_COMPILER)
			std::string shader_source{};
			if (!filesystem->ReadFile(stage.path, shader_source))
			{
				EDGE_LOGE("Failed to open shader: {}.", stage.path);
				return {};
			}

			ShaderCompileInfo shader_compile_info{};
			shader_compile_info.path = stage.path;
			shader_compile_info.definitions = stage.definitions;
			shader_compile_info.source = shader_source;

			uint32_t shader_hash = shader_hash = murmur_hash(shader_source, 0);
			uint32_t dependency_hash = shader_compiler->CollectDependencyHash(shader_compile_info);
#endif

			// Return found shader code
			auto found = shaders.find(unique_shader_key);
			if (found != shaders.end() && 
				found->second.shader_hash == shader_hash &&
				found->second.dependency_hash == dependency_hash)
			{
				EDGE_LOGI("Loaded shader: {}.", stage.path);
				return found->second.code;
			}

			auto& new_shader = shaders[unique_shader_key];
			new_shader.shader_hash = shader_hash;
			new_shader.dependency_hash = dependency_hash;
			if (shader_compiler->Compile(shader_compile_info, new_shader.code))
			{
				EDGE_LOGI("Compiled shader: {}.", stage.path);
				return new_shader.code;
			}

			EDGE_LOGE("Failed to compile shader: {}.", stage.path);

			return {};
		}
	}
}