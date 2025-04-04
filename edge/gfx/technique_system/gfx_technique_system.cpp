#include "gfx_technique_system.h"

#include "gfx_glsl_shader_compiler.h"

#include "../gfx_context.h"

#include "../../fs/fs_helper.h"
#include "../../util/Logger.h"
#include "../../fs/fs_virtual_filesystem.h"

// TODO: define it from specific config
#define DEVELOPMENT

namespace edge::gfx
{
	NLOHMANN_JSON_SERIALIZE_ENUM(TechniqueType,
		{
			{TechniqueType::eNone, nullptr},
			{TechniqueType::eCompute, "compute"},
			{TechniqueType::eGraphics, "graphics"}
		})

	constexpr const std::string_view k_frame_graph_cache_path = "/internal/fgcache";

	TechniqueDatabase::TechniqueDatabase() {
		ShaderCompilerCreateInfo compiler_ci{};
		//TODO: get this data from database ci
		compiler_ci.base_path = "/internal_res/assets/shaders";
		compiler_ci.additional_include_dirs = { "includes" };

		shader_compiler = std::make_unique<GLSLShaderCompiler>(compiler_ci);

		std::vector<uint8_t> pipeline_cache_data{};
		if (fs::exists(k_frame_graph_cache_path) && fs::read_file(k_frame_graph_cache_path, pipeline_cache_data)) {
		}

		PipelineCacheCreateInfo pipeline_cache_ci{};
		pipeline_cache_ci.data = pipeline_cache_data;

		pipeline_cache = create_pipeline_cache(pipeline_cache_ci);
	}

	TechniqueDatabase::~TechniqueDatabase() {
		for (auto& [name, technique] : techniques) {
			destroy(technique.pipeline);
			destroy(technique.root_signature);
			destroy(technique.shader);
		}

		techniques.clear();

		size_t cache_size{ 0 };
		pipeline_cache->get_data(cache_size, nullptr);

		if (cache_size) {
			std::vector<uint8_t> data(cache_size, '\0');
			pipeline_cache->get_data(cache_size, data.data());

			fs::write_file(k_frame_graph_cache_path, data);
		}

		destroy(pipeline_cache);
		pipeline_cache = nullptr;
	}

	template<typename _Ty>
	void try_parse_to(std::string_view name, const nlohmann::json& j, _Ty& o) {
		if (auto obj = j.find(name); obj != j.end()) {
			obj->get_to(o);
		}
	}

	bool TechniqueDatabase::load_config(const TechniqueConfigInfo& config) {
		if (!fs::exists(config.db_path)) {
			EDGE_LOGE("[TechniqueSystem] Configuration file \"{}\" is not exists.", config.db_path);
			return false;
		}

		std::vector<uint8_t> file_data;
		if (!fs::read_file(config.db_path, file_data)) {
			EDGE_LOGE("[TechniqueSystem] Failed to read configuration file \"{}\".", config.db_path);
			return false;
		}

		auto json = nlohmann::json::parse(file_data);

		if (!json.is_array()) {
			EDGE_LOGE("[TechniqueSystem] Configuration file \"{}\" is not contain array with techniques.", config.db_path);
			return false;
		}

		for (const auto& technique : json) {
			TechniqueDesc technique_desc;
			technique_desc.type = technique["type"].get<TechniqueType>();
			technique_desc.shared_descriptor = config.shared_descriptor;

			// Parse stages
			if (auto stages = technique.find("stages"); stages != technique.end()) {
				for (const auto& stage : *stages) {
					auto& new_stage = technique_desc.stages.emplace_back();
					try_parse_to("path", stage, new_stage.path);
					try_parse_to("def", stage, new_stage.definitions);
				}
			}

			// Parse vertex layout
			if (auto vertex_layout = technique.find("vertex_layout"); vertex_layout != technique.end()) {
				try_parse_to("attributes", *vertex_layout, technique_desc.vertex_layout.attributes);
				try_parse_to("bindings", *vertex_layout, technique_desc.vertex_layout.bindings);
			}

			// Parse color attachments and color blending data
			if (auto attachments = technique.find("attachments"); attachments != technique.end()) {
				for (const auto& attachment : *attachments) {
					auto format = attachment["format"].get<Format>();

					if (format == Format::eUndefined) {
						format = config.backbuffer_format;
					}

					technique_desc.color_formats.emplace_back(format);

					auto& color_blending = technique_desc.color_blending_attachments.emplace_back();

					try_parse_to("blending", attachment, color_blending.enable);
					try_parse_to("src_color", attachment, color_blending.src_color);
					try_parse_to("dst_color", attachment, color_blending.dst_color);
					try_parse_to("color_blend_op", attachment, color_blending.color_blend_op);
					try_parse_to("src_alpha", attachment, color_blending.src_alpha);
					try_parse_to("dst_alpha", attachment, color_blending.dst_alpha);
					try_parse_to("alpha_blend_op", attachment, color_blending.alpha_blend_op);

					if (auto color_mask = attachment.find("color_mask"); color_mask != attachment.end()) {
						for (const auto& mask_flag : *color_mask) {
							color_blending.color_mask |= mask_flag.get<ColorMaskFlag>();
						}
					}
				}
			}

			// Parse rasterization state
			if (auto rasterization = technique.find("rasterization"); rasterization != technique.end()) {
				try_parse_to("depth_clamp", *rasterization, technique_desc.rasterization_state.depth_clamp);
				try_parse_to("discard_enable", *rasterization, technique_desc.rasterization_state.discard_enable);
				try_parse_to("fill_mode", *rasterization, technique_desc.rasterization_state.fill_mode);

				if (auto cull_mode_mask = rasterization->find("cull_mode"); cull_mode_mask != rasterization->end()) {
					for (const auto& cull_mode_flag : *cull_mode_mask) {
						CullModeFlag flag = cull_mode_flag.get<CullModeFlag>();
						technique_desc.rasterization_state.cull_mode |= flag;
					}
				}

				try_parse_to("front_face", *rasterization, technique_desc.rasterization_state.front_face);
			}

			// Parse multisampling state
			if (auto multisampling = technique.find("multisampling"); multisampling != technique.end()) {
				try_parse_to("sample_count", *multisampling, technique_desc.multisample_state.sample_count);
				try_parse_to("alpha_to_coverage", *multisampling, technique_desc.multisample_state.alpha_to_coverage);
				try_parse_to("alpha_to_one", *multisampling, technique_desc.multisample_state.alpha_to_one);
			}

			// Parse depth stencil state
			if (auto depth_stencil = technique.find("depth_stencil"); depth_stencil != technique.end()) {
				try_parse_to("depth_format", *depth_stencil, technique_desc.depth_format);
				try_parse_to("stencil_format", *depth_stencil, technique_desc.stencil_format);

				try_parse_to("depth_test", *depth_stencil, technique_desc.depth_stencil_state.depth_test);
				try_parse_to("depth_write", *depth_stencil, technique_desc.depth_stencil_state.depth_write);
				try_parse_to("depth_compare_op", *depth_stencil, technique_desc.depth_stencil_state.depth_compare_op);

				try_parse_to("stencil_test", *depth_stencil, technique_desc.depth_stencil_state.stencil_test);
				try_parse_to("stencil_front_compare_op", *depth_stencil, technique_desc.depth_stencil_state.stencil_front_compare_op);
				try_parse_to("front_fail", *depth_stencil, technique_desc.depth_stencil_state.front_fail);
				try_parse_to("depth_front_fail", *depth_stencil, technique_desc.depth_stencil_state.depth_front_fail);
				try_parse_to("front_pass", *depth_stencil, technique_desc.depth_stencil_state.front_pass);

				try_parse_to("stencil_back_compare_op", *depth_stencil, technique_desc.depth_stencil_state.stencil_back_compare_op);
				try_parse_to("back_fail", *depth_stencil, technique_desc.depth_stencil_state.back_fail);
				try_parse_to("depth_back_fail", *depth_stencil, technique_desc.depth_stencil_state.depth_back_fail);
				try_parse_to("back_pass", *depth_stencil, technique_desc.depth_stencil_state.back_pass);
			}

			try_parse_to("primitive_topology", technique, technique_desc.primitive_topology);

			std::string technique_name = technique["name"].get<std::string>();
			add_technique(technique_name, technique_desc);
		}

		return true;
	}

	bool TechniqueDatabase::load(const std::string& path) {
		if (!fs::exists(path))
			return false;

		auto* file = fs::open_file(path, fs::FileModeFlag::eRead | fs::FileModeFlag::eBin);

		size_t shader_count;
		file->read(shader_count);

		for (int32_t i = 0; i < shader_count; ++i) {
			uint32_t hash;
			file->read(hash);

			auto& shader = shaders[hash];

			size_t code_size;
			file->read(code_size);

			shader.code.resize(code_size);
			file->read(shader.code);
			file->read(shader.shader_hash);
			file->read(shader.dependency_hash);
		}

		delete file;

		return true;
	}

	bool TechniqueDatabase::save(const std::string& path) {
		auto* file = fs::open_file(path, fs::FileModeFlag::eWrite | fs::FileModeFlag::eBin);

		size_t shader_count = shaders.size();

		// Write shader_count first
		file->write(shader_count);

		for (const auto& [hash, shader] : shaders)
		{
			file->write(hash);

			file->write(shader.code.size());
			file->write(shader.code);
			file->write(shader.shader_hash);
			file->write(shader.dependency_hash);
		}

		delete file;

		return true;
	}

	void TechniqueDatabase::add_technique(const std::string& name, const TechniqueDesc& technique_desc) {
		std::vector<ShaderStageInfo> shader_stages{};

		if (technique_desc.type == TechniqueType::eNone) {
			EDGE_LOGE("Incorrect technique type for technique named: {}.", name);
			return;
		}

		for (auto& stage : technique_desc.stages) {
			ShaderStageFlag stage_flag{};

			auto ext = fs::get_ext(stage.path);
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
				.code = get_or_compile_shader(stage)
				});
		}

		IShader* shader = create_shader(shader_stages);
		shader->set_resource_name(name);

		RootSignatureCreateInfo root_sign_ci{};
		root_sign_ci.shader = shader;
		root_sign_ci.shared_descriptor = technique_desc.shared_descriptor;
		IRootSignature* root_signature = create_root_signature(root_sign_ci);
		root_signature->set_resource_name(name);

		IPipeline* pipeline{ nullptr };
		if (technique_desc.type == TechniqueType::eCompute) {
			ComputePipelineCreateInfo pipeline_ci{};
			pipeline_ci.shader = shader;
			pipeline_ci.root_signature = root_signature;
			pipeline_ci.pipeline_cache = pipeline_cache;
			pipeline = create_pipeline(pipeline_ci);
		}
		else if (technique_desc.type == TechniqueType::eGraphics) {
			GraphicsPipelineCreateInfo pipeline_ci{};
			pipeline_ci.shader = shader;
			pipeline_ci.root_signature = root_signature;
			pipeline_ci.pipeline_cache = pipeline_cache;

			for (const auto& attribute : technique_desc.vertex_layout.attributes) {
				pipeline_ci.pipeline_state.vertex_layout.attributes[pipeline_ci.pipeline_state.vertex_layout.attribute_count++] = attribute;
			}

			for (const auto& binding : technique_desc.vertex_layout.bindings) {
				pipeline_ci.pipeline_state.vertex_layout.bindings[pipeline_ci.pipeline_state.vertex_layout.binding_count++] = binding;
			}

			for (const auto& attachment : technique_desc.color_blending_attachments) {
				pipeline_ci.pipeline_state.color_blending_state.attachments[pipeline_ci.pipeline_state.color_blending_state.attachment_count++] = attachment;
			}

			pipeline_ci.pipeline_state.rasterization_state = technique_desc.rasterization_state;
			pipeline_ci.pipeline_state.multisample_state = technique_desc.multisample_state;
			pipeline_ci.pipeline_state.depth_stencil_state = technique_desc.depth_stencil_state;
			pipeline_ci.pipeline_state.primitive_topology = technique_desc.primitive_topology;

			for (Format format : technique_desc.color_formats) {
				pipeline_ci.color_formats[pipeline_ci.color_format_count++] = format;
			}

			pipeline_ci.depth_format = technique_desc.depth_format;
			pipeline_ci.stencil_format = technique_desc.stencil_format;
			pipeline = create_pipeline(pipeline_ci);
		}

		pipeline->set_resource_name(name);

		techniques.emplace(name, Technique{ name, shader, root_signature, pipeline });
	}

	auto TechniqueDatabase::get_technique(const std::string& name) -> Technique& {
		static Technique fallback_technique{};

		auto found = techniques.find(name);
		if (found != techniques.end()) {
			return found->second;
		}

		EDGE_LOGE("[Technique Database] Rendering technique with name \"{}\" was not found.", name);

		return fallback_technique;
	}

	std::span<const uint8_t> TechniqueDatabase::get_or_compile_shader(const TechniqueDesc::ShaderStage& stage) {
		// This hash key needed to keep some cache for compiled shaders
		uint32_t unique_shader_key{ 0u };
		unique_shader_key ^= murmur_hash(stage.path);

		for (const auto& define : stage.definitions)
			unique_shader_key ^= murmur_hash(define);

#if defined(DEVELOPMENT) || defined(EMBEDDED_SHADER_COMPILER)
		std::string shader_source{};
		fs::read_file(stage.path, shader_source);

		ShaderCompileInfo shader_compile_info{};
		shader_compile_info.path = stage.path;
		shader_compile_info.definitions = std::move(stage.definitions);
		shader_compile_info.source = shader_source;

		uint32_t shader_hash = shader_hash = murmur_hash(shader_source, 0);
		uint32_t dependency_hash = shader_compiler->collect_dependency_hash(shader_compile_info);
#endif

		// Return found shader code
		auto found = shaders.find(unique_shader_key);
		if (found != shaders.end() &&
			found->second.shader_hash == shader_hash &&
			found->second.dependency_hash == dependency_hash) {
			EDGE_LOGD("Loaded shader: {}.", stage.path);
			return found->second.code;
		}

		auto& new_shader = shaders[unique_shader_key];
		new_shader.shader_hash = shader_hash;
		new_shader.dependency_hash = dependency_hash;
		if (shader_compiler->compile(shader_compile_info, new_shader.code)) {
			EDGE_LOGD("Compiled shader: {}.", stage.path);
			return new_shader.code;
		}

		EDGE_LOGE("Failed to compile shader: {}.", stage.path);

		return {};
	}
}