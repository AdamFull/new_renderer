#include "gfx_shader_effect_compiler.h"
#include <serialization.hpp>

#include "gfx_shader_effect.h"

#include "glsl_shader_compiler.h"

#include <util/FilesystemHelper.h>
#include <util/Logger.h>

namespace engine
{
	namespace gfx
	{
		namespace shc
		{
			struct ShaderEffectCompileSourceDesc
			{
				std::string source;
				ShaderStageFlag type;
				std::vector<std::string> definitions{};
			};

			void to_json(nlohmann::json& json, const ShaderEffectCompileSourceDesc& type)
			{
				utl::serialize_to("source", json, type.source);
				utl::serialize_to("type", json, type.type);
				utl::serialize_to("definitions", json, type.definitions);
			}

			void from_json(const nlohmann::json& json, ShaderEffectCompileSourceDesc& type)
			{
				utl::parse_from("source", json, type.source, true);
				utl::parse_from("type", json, type.type, true);
				utl::parse_from("definitions", json, type.definitions);
			}

			struct ShaderEffectCompileSamplerDesc
			{
				std::string binding;
				SamplerCreateInfo sampler_create_info;
			};

			void to_json(nlohmann::json& json, const ShaderEffectCompileSamplerDesc& type)
			{
				utl::serialize_to("binding", json, type.binding);

				utl::serialize_to("address_u", json, type.sampler_create_info.address_u);
				utl::serialize_to("address_v", json, type.sampler_create_info.address_v);
				utl::serialize_to("address_w", json, type.sampler_create_info.address_w);

				utl::serialize_to("min_filter", json, type.sampler_create_info.min_filter);
				utl::serialize_to("mag_filter", json, type.sampler_create_info.mag_filter);
				utl::serialize_to("mip_filter", json, type.sampler_create_info.mip_map_mode);

				utl::serialize_to("mip_lod_bias", json, type.sampler_create_info.mip_lod_bias);
				utl::serialize_to("min_lod", json, type.sampler_create_info.min_lod);
				utl::serialize_to("max_lod", json, type.sampler_create_info.max_lod);
				utl::serialize_to("max_anisotropy", json, type.sampler_create_info.max_anisotropy);

				utl::serialize_to("compare_op", json, type.sampler_create_info.depth_compare);

			}

			void from_json(const nlohmann::json& json, ShaderEffectCompileSamplerDesc& type)
			{
				utl::parse_from("binding", json, type.binding, true);

				utl::parse_from("address_u", json, type.sampler_create_info.address_u);
				utl::parse_from("address_v", json, type.sampler_create_info.address_v);
				utl::parse_from("address_w", json, type.sampler_create_info.address_w);

				utl::parse_from("min_filter", json, type.sampler_create_info.min_filter);
				utl::parse_from("mag_filter", json, type.sampler_create_info.mag_filter);
				utl::parse_from("mip_filter", json, type.sampler_create_info.mip_map_mode);

				utl::parse_from("mip_lod_bias", json, type.sampler_create_info.mip_lod_bias);
				utl::parse_from("min_lod", json, type.sampler_create_info.min_lod);
				utl::parse_from("max_lod", json, type.sampler_create_info.max_lod);
				utl::parse_from("max_anisotropy", json, type.sampler_create_info.max_anisotropy);

				utl::parse_from("compare_op", json, type.sampler_create_info.depth_compare);
			}

			struct ShaderEffectCompilePipelineState
			{
				RasterizationState rasterization_state{};
				MultisampleState multisample_state{};
				DepthStencilState depth_stencil_state{};
				ColorBlendingState color_blending_state{};

				PrimitiveTopology primitive_topology{ PrimitiveTopology::eTriangleList };
			};

			void to_json(nlohmann::json& json, const ShaderEffectCompilePipelineState& type)
			{
				utl::serialize_to("rasterization_state", json, type.rasterization_state);
				utl::serialize_to("multisample_state", json, type.multisample_state);
				utl::serialize_to("depth_stencil_state", json, type.depth_stencil_state);
				utl::serialize_to("color_blending_state", json, type.color_blending_state);
			}

			void from_json(const nlohmann::json& json, ShaderEffectCompilePipelineState& type)
			{
				utl::parse_from("rasterization_state", json, type.rasterization_state);
				utl::parse_from("multisample_state", json, type.multisample_state);
				utl::parse_from("depth_stencil_state", json, type.depth_stencil_state);
				utl::parse_from("color_blending_state", json, type.color_blending_state);
			}

			struct ShaderEffectCompileDesc
			{
				std::string name;
				std::vector<ShaderEffectCompileSourceDesc> shaders{};
				std::vector<ShaderEffectCompileSamplerDesc> samplers{};
				ShaderEffectCompilePipelineState pipeline_state{};
			};

			void to_json(nlohmann::json& json, const ShaderEffectCompileDesc& type)
			{
				utl::serialize_to("name", json, type.name);
				utl::serialize_to("shaders", json, type.shaders);
				utl::serialize_to("samplers", json, type.samplers);
				utl::serialize_to("pipeline_state", json, type.pipeline_state);
			}

			void from_json(const nlohmann::json& json, ShaderEffectCompileDesc& type)
			{
				utl::parse_from("name", json, type.name);
				utl::parse_from("shaders", json, type.shaders);
				utl::parse_from("samplers", json, type.samplers);
				utl::parse_from("pipeline_state", json, type.pipeline_state);
			}


			void to_json(nlohmann::json& json, const ShaderEffectCompilerPermutation& type)
			{
				utl::serialize_to("definitions", json, type.definitions);
			}

			void from_json(const nlohmann::json& json, ShaderEffectCompilerPermutation& type)
			{
				utl::parse_from("definitions", json, type.definitions);
			}


			void to_json(nlohmann::json& json, const ShaderEffectCompilerModule& type)
			{
				utl::serialize_to("name", json, type.name);
				utl::serialize_to("path", json, type.path);
				utl::serialize_to("include", json, type.include);
				utl::serialize_to("exclude", json, type.exclude);
			}

			void from_json(const nlohmann::json& json, ShaderEffectCompilerModule& type)
			{
				utl::parse_from("name", json, type.name);
				utl::parse_from("path", json, type.path);
				utl::parse_from("include", json, type.include);
				utl::parse_from("exclude", json, type.exclude);
			}


			void to_json(nlohmann::json& json, const ShaderEffectCompilerConfig& type)
			{
				utl::serialize_to("lang", json, type.lang);
				utl::serialize_to("include_directories", json, type.include_directories);
				utl::serialize_to("common_directories", json, type.common_directories);
				utl::serialize_to("permutation_patterns", json, type.permutation_patterns);
				utl::serialize_to("modules", json, type.modules);
			}

			void from_json(const nlohmann::json& json, ShaderEffectCompilerConfig& type)
			{
				utl::parse_from("lang", json, type.lang);
				utl::parse_from("include_directories", json, type.include_directories);
				utl::parse_from("common_directories", json, type.common_directories);
				utl::parse_from("permutation_patterns", json, type.permutation_patterns);
				utl::parse_from("modules", json, type.modules);
			}

			ShaderEffectCompiler::ShaderEffectCompiler(const ShaderEffectCompilerCreateInfo& create_info) :
				fs{ *create_info.filesystem_wrapper },
				build_type{ create_info.build_type }
			{
				// Load compiler config
				std::vector<uint8_t> filedata{};
				if (!create_info.shader_compile_config_path.empty() &&
					fs.Exists(create_info.shader_compile_config_path) && 
					fs.ReadFile(create_info.shader_compile_config_path, filedata))
				{
					// Load configuration
					auto json = nlohmann::json::parse(filedata);
					json.get_to(config);

					is_config_loaded = true;
					base_path = fsh::parent_path(create_info.shader_compile_config_path);

					switch (config.lang)
					{
					case ShaderLang::eAuto: { assert(false && "NOT IMPLEMENTED"); } break;
					case ShaderLang::eGLSL: { shader_compiler = std::make_unique<GLSLShaderCompiler>(); } break;
					case ShaderLang::eHLSL: { assert(false && "NOT IMPLEMENTED"); } break;
					}

					filedata.clear();
				}

				// Load compiler cache
				if (!create_info.shader_cache_patch.empty() &&
					fs.Exists(create_info.shader_cache_patch) && 
					fs.ReadFile(create_info.shader_cache_patch, filedata))
				{

				}

				CompileModules();
			}

			ShaderEffectCompiler::~ShaderEffectCompiler()
			{

			}

			void ShaderEffectCompiler::CompileModules()
			{
				if (!is_config_loaded)
				{
					log_error("Compiler config was not loaded. Unable to compile modules.");
					return;
				}

				std::array<char, 1024ull> str_buf{};

				CompileShaderInfo compile_info{};
				compile_info.build_type = build_type;
				compile_info.include_directories = config.include_directories;

				for (const auto& compile_module : config.modules)
				{
					log_info("Building module: {}", compile_module.name);

					uint32_t module_name_hash = utl::murmur_hash(compile_module.name);

					auto module_path = fsh::path_append(base_path, compile_module.path, str_buf);

					// This one can run parallel?
					auto list_dir = fs.ListDir(module_path, compile_module.include, compile_module.exclude);
					for (const auto& effect_path : list_dir)
					{
						// Read shader effect description file
						std::vector<uint8_t> file_data{};
						if (fs.ReadFile(effect_path, file_data))
						{
							file_data.clear();

							ShaderEffectCompileDesc desc;

							auto json = nlohmann::json::parse(file_data);
							json.get_to(desc);

							std::array<char, 1024ull> path_buffer{};

							// Iterate each shader description
							for (const auto& shader_source : desc.shaders)
							{
								// TODO: check in shader cache
								auto shader_path = fsh::path_append(module_path, shader_source.source, path_buffer);
								if (fs.ReadFile(shader_path, file_data))
								{

								}
							}

							continue;
						}
					}
				}
			}

			void ShaderEffectCompiler::CompileOne(const char* shader_name)
			{
				if (!is_config_loaded)
				{
					log_error("Compiler config was not loaded. Unable to compile shaders.");
					return;
				}
			}
		}
	}
}