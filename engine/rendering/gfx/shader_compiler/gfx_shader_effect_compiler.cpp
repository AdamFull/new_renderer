#include "gfx_shader_effect_compiler.h"
#include <serialization.hpp>

#include "gfx_shader_effect.h"

#include "glsl_shader_compiler.h"
#include "../gfx_util.h"

#include <util/FilesystemHelper.h>
#include <util/Logger.h>

#include <execution>

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

			struct ShaderEffectCompileRules
			{
				uint32_t shader_model{ 6u };
				std::string permutation_pattern{};
				std::vector<std::vector<std::string>> permutate{};
			};

			void to_json(nlohmann::json& json, const ShaderEffectCompileRules& type)
			{
				utl::serialize_to("shader_model", json, type.shader_model);
				utl::serialize_to("permutation_pattern", json, type.permutation_pattern);
				utl::serialize_to("permutate", json, type.permutate);
			}

			void from_json(const nlohmann::json& json, ShaderEffectCompileRules& type)
			{
				utl::parse_from("shader_model", json, type.shader_model);
				utl::parse_from("permutation_pattern", json, type.permutation_pattern);
				utl::parse_from("permutate", json, type.permutate);
			}

			struct ShaderEffectCompileDesc
			{
				std::string name;
				std::vector<ShaderEffectCompileSourceDesc> shaders{};
				std::vector<ShaderEffectCompileSamplerDesc> samplers{};
				ShaderEffectCompilePipelineState pipeline_state{};
				ShaderEffectCompileRules compile{};
			};

			void to_json(nlohmann::json& json, const ShaderEffectCompileDesc& type)
			{
				utl::serialize_to("name", json, type.name);
				utl::serialize_to("shaders", json, type.shaders);
				utl::serialize_to("samplers", json, type.samplers);
				utl::serialize_to("pipeline_state", json, type.pipeline_state);
				utl::serialize_to("compile", json, type.compile);
			}

			void from_json(const nlohmann::json& json, ShaderEffectCompileDesc& type)
			{
				utl::parse_from("name", json, type.name);
				utl::parse_from("shaders", json, type.shaders);
				utl::parse_from("samplers", json, type.samplers);
				utl::parse_from("pipeline_state", json, type.pipeline_state);
				utl::parse_from("compile", json, type.compile);
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


			void to_json(nlohmann::json& json, const CachedShaderDesc& type)
			{
				utl::serialize_to("stage", json, type.stage);
				utl::serialize_to("shader_key", json, type.shader_key);
			}

			void from_json(const nlohmann::json& json, CachedShaderDesc& type)
			{
				utl::parse_from("stage", json, type.stage);
				utl::parse_from("shader_key", json, type.shader_key, true);
			}


			void to_json(nlohmann::json& json, const ShaderEffectDesc& type)
			{
				utl::serialize_to("shader_descriptions", json, type.shader_descriptions);
			}

			void from_json(const nlohmann::json& json, ShaderEffectDesc& type)
			{
				utl::parse_from("shader_descriptions", json, type.shader_descriptions);
			}


			void to_json(nlohmann::json& json, const ShaderCache& type)
			{
				utl::serialize_to("shaders", json, type.shaders);
				utl::serialize_to("modules", json, type.modules);
			}

			void from_json(const nlohmann::json& json, ShaderCache& type)
			{
				utl::parse_from("shaders", json, type.shaders);
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
					output_path = fsh::parent_path(create_info.shader_cache_patch);

					switch (config.lang)
					{
					case ShaderLang::eAuto: { assert(false && "NOT IMPLEMENTED"); } break;
					case ShaderLang::eGLSL: { shader_compiler = std::make_unique<GLSLShaderCompiler>(fs); } break;
					case ShaderLang::eHLSL: { assert(false && "NOT IMPLEMENTED"); } break;
					}

					filedata.clear();
				}

				// Load compiler cache
				if (!create_info.shader_cache_patch.empty() &&
					fs.Exists(create_info.shader_cache_patch) && 
					fs.ReadFile(create_info.shader_cache_patch, filedata))
				{
					auto json = nlohmann::json::from_bson(filedata.begin(), filedata.end());
					json.get_to(shader_cache);
				}

				CompileModules();
			}

			ShaderEffectCompiler::~ShaderEffectCompiler()
			{

			}

			void ShaderEffectCompiler::CompileModules()
			{
				bool is_compiled_something{ false };
				if (!is_config_loaded)
				{
					log_error("Compiler config was not loaded. Unable to compile modules.");
					return;
				}

				std::array<char, 256ull> str_buf{};

				for (const auto& compile_module : config.modules)
				{
					log_info("Building module: {}", compile_module.name);

					auto out_module_path = output_path + '/' + compile_module.name;
					if (!fs.Exists(out_module_path))
					{
						fs.Mkdir(out_module_path);
					}

					auto module_path = fsh::path_append(base_path, compile_module.path, str_buf);

					// This one can run parallel?
					auto list_dir = fs.ListDir(module_path, compile_module.include, compile_module.exclude);
					for (const auto& effect_path : list_dir)
					{
						// Read shader effect description file
						std::vector<uint8_t> file_data{};
						if (!fs.ReadFile(effect_path, file_data))
							continue;

						auto json = nlohmann::json::parse(file_data);
						file_data.clear();

						ShaderEffectCompileDesc desc;
						json.get_to(desc);

						uint32_t shader_souece_index{ 0u };

						// Iterate each shader description
						for (const auto& shader_source : desc.shaders)
						{
							std::string shader_code{};

							// TODO: check in shader cache
							std::array<char, 256ull> path_buffer{};
							auto shader_path = fsh::path_append(module_path, shader_source.source, path_buffer);
							if (!fs.Exists(shader_path))
							{
								// Try to find in common
								for (const auto& common_dir : config.common_directories)
								{
									shader_path = fsh::path_append(base_path, common_dir, path_buffer);
									shader_path = fsh::path_append(shader_path, shader_source.source, path_buffer);
									if (fs.Exists(shader_path))
										break;
								}
							}

							if (!fs.ReadFile(shader_path, shader_code))
								continue;

							CompileShaderInfo compile_info{};
							compile_info.build_type = build_type;
							compile_info.shader_index = shader_souece_index++;

							compile_info.base_path = base_path;

							compile_info.module_name = compile_module.name;
							compile_info.name = desc.name;

							compile_info.stage = shader_source.type;

							compile_info.base_definitions = shader_source.definitions;

							compile_info.include_directories = config.include_directories;

							if (!desc.compile.permutation_pattern.empty())
							{
								if (auto found = config.permutation_patterns.find(desc.compile.permutation_pattern); found != config.permutation_patterns.end())
								{
									for (const auto& permutation : found->second)
									{
										compile_info.definitions = permutation;
										is_compiled_something = CompilePermutation(compile_info, shader_code) || is_compiled_something;
									}
								}
							}

							if (!desc.compile.permutate.empty())
							{
								for (const auto& permutation : desc.compile.permutate)
								{
									compile_info.definitions = permutation;
									is_compiled_something = CompilePermutation(compile_info, shader_code) || is_compiled_something;
								}
							}

							if (desc.compile.permutation_pattern.empty() && desc.compile.permutate.empty())
							{
								is_compiled_something = CompilePermutation(compile_info, shader_code) || is_compiled_something;
							}
						}
					}
				}

				// Save cache if needed
				if (is_compiled_something)
				{
					auto json = nlohmann::json(shader_cache);
					std::vector<uint8_t> data = nlohmann::json::to_bson(json);
					if (!fs.WriteFile(output_path + "/shader_cache.shc", data))
					{
						log_error("Failed to save shader cache!");
					}
				}
			}

			uint32_t GetUniqueHashName(std::string_view input)
			{
				uint32_t hash{ 0u };

				// Current substring start index
				size_t start = 0;
				while (start < input.size())
				{
					// Find the next ':' separator or the end of the string
					size_t end = input.find(':', start);

					// Extract the current substring as a view
					std::string_view substring = input.substr(start, (end == std::string_view::npos) ? input.size() - start : end - start);

					// Combine the hash of the substring into the total hash
					hash += utl::murmur_hash(substring);

					// Move to the next substring
					if (end == std::string_view::npos) break;
					start = end + 1;
				}

				return hash;
			}

			ShaderEffectCreateInfo ShaderEffectCompiler::GetEffect(std::string_view shader_name)
			{
				std::array<char, 256ull> buffer{};

				ShaderEffectCreateInfo effect_create_info{};

				// Lookup for shaders
				auto unique_name_hash = GetUniqueHashName(shader_name);
				auto found_effect = shader_cache.modules.find(unique_name_hash);
				if (found_effect == shader_cache.modules.end())
					return effect_create_info;
				
				auto& shader_effect = found_effect->second;

				// Now we can find all needed shaders
				for (const auto& shader_desc : shader_effect.shader_descriptions)
				{
					auto found_shader = shader_cache.shaders.find(shader_desc.shader_key);
					if (found_shader == shader_cache.shaders.end())
						continue;

					auto& new_shader_stage = effect_create_info.shader_stages.emplace_back();
					new_shader_stage.stage = shader_desc.stage;

					auto shader_path = fsh::path_append(output_path, found_shader->second, buffer);

					// Read shader code
					if (!fs.ReadFile(shader_path, new_shader_stage.code))
						continue;
				}
				
				return effect_create_info;
			}

			void ShaderEffectCompiler::CompileOne(const char* shader_name)
			{
				if (!is_config_loaded)
				{
					log_error("Compiler config was not loaded. Unable to compile shaders.");
					return;
				}
			}

			uint32_t GenerateUniqueModuleHashName(const CompileShaderInfo& compile_info)
			{
				uint32_t hash = utl::murmur_hash(compile_info.module_name);
				hash += utl::murmur_hash(compile_info.name);

				for (const auto& definition : compile_info.definitions)
					hash += utl::murmur_hash(definition);

				return hash;
			}

			bool ShaderEffectCompiler::CompilePermutation(const CompileShaderInfo& compile_info, std::string_view shader_code)
			{
				bool is_first_time_compile{ false };
				uint32_t unique_shader_permutation_name_hash = GenerateUniqueModuleHashName(compile_info);

				// Get shader stabge bank
				auto& shader_module = shader_cache.modules[unique_shader_permutation_name_hash];

				uint32_t shader_permutation_hash{ 0u };
				std::string preprocessed_code{};
				if (!shader_compiler->Preprocess(compile_info, shader_code, preprocessed_code, shader_permutation_hash))
				{
					log_error("Failed to preprocess shader: {}", compile_info.name);
					return false;
				}

				if (shader_module.shader_descriptions.empty() || (shader_module.shader_descriptions.size() <= compile_info.shader_index))
				{
					shader_module.shader_descriptions.emplace_back(compile_info.stage, 0u);
					is_first_time_compile = true;
				}

				auto& current_desc = shader_module.shader_descriptions[compile_info.shader_index];

				bool is_code_outdated = current_desc.shader_key != shader_permutation_hash;
				bool is_type_missmatch = current_desc.stage != compile_info.stage;

				if (is_type_missmatch)
				{
					//log_warning("Type mismatch for shader module named: \"{}\". Previous type: \"{}\" new type: \"{}\". Old shaders will be removed.",
					//	unique_module_name, get_shader_type_str(current_module.type), get_shader_type_str(shader_meta.type));

					// Removing old shaders
					for (auto& shmodule : shader_module.shader_descriptions)
						shader_cache.shaders.erase(shmodule.shader_key);

					// Cleaning links
					shader_module.shader_descriptions.clear();
					current_desc = shader_module.shader_descriptions.emplace_back(compile_info.stage, 0u);
				}

				// Compile shader if we need
				if (is_code_outdated || is_type_missmatch || is_first_time_compile)
				{
					auto& compiled_shader_path = shader_cache.shaders[shader_permutation_hash];

					std::vector<uint8_t> compiled_spirv{};
					if (!shader_compiler->Compile(compile_info, preprocessed_code, compiled_spirv))
						return false;

					// Inform that shader is compiled
					{
						std::stringstream ss{};
						ss << compile_info.name;
						for (auto& definition : compile_info.definitions)
							ss << ":" << definition;

						log_info("{} shader {}[{}] was {}.", unique_shader_permutation_name_hash, ss.str(), shader_permutation_hash, is_first_time_compile ? "compiled" : "recompiled");
					}

					// Generate name
					{
						std::stringstream ss;
						ss << compile_info.module_name << "/" << compile_info.name << "." << utl::enum_to_string(compile_info.stage) << "." << unique_shader_permutation_name_hash << ".spv";
						compiled_shader_path = ss.str();
					}

					// Save compiled shader
					{
						std::stringstream ss;
						ss << output_path << '/' << compiled_shader_path;
						if (!fs.WriteFile(ss.str(), compiled_spirv))
						{
							log_error("Failed to comile shader: {}.", compiled_shader_path);
							return false;
						}
					}
				}

				// Removing old shader data
				if ((is_code_outdated || is_type_missmatch) && !is_first_time_compile)
					shader_cache.shaders.erase(current_desc.shader_key);

				current_desc.shader_key = shader_permutation_hash;

				return true;
			}
		}
	}
}