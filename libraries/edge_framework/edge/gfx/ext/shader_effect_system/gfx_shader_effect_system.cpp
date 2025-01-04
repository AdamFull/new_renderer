#include "gfx_shader_effect_system.h"

#include "../../gfx_context.h"
#include "gfx_shader_effect_compiler.h"

namespace edge
{
	namespace gfx
	{
		namespace ext
		{
			int32_t GetShaderIndex(ShaderStageFlag stage)
			{
				if (stage == ShaderStageFlag::eVertex)
					return 0;
				else if (stage == ShaderStageFlag::eTessellationControl)
					return 1;
				else if (stage == ShaderStageFlag::eTessellationEvaluation)
					return 2;
				else if (stage == ShaderStageFlag::eGeometry)
					return 3;
				else if (stage == ShaderStageFlag::eFragment)
					return 4;
				else if (stage == ShaderStageFlag::eCompute)
					return 5;

				return -1;
			}

			class ShaderEffectImpl final : public ShaderEffect
			{
			public:
				ShaderEffectImpl(Context& context, ShaderEffectCreateInfo create_info) :
					context{ context }
				{
					if (create_info.shader_stages.empty())
						return;

					std::vector<ShaderStageInfo> shader_stages;
					for (const auto& shader_stage : create_info.shader_stages)
					{
						shader_stages.emplace_back(
							ShaderStageInfo
							{
								.stage = shader_stage.stage,
								.code = shader_stage.code
							}
						);
					}

					shader = context.CreateShader(shader_stages);
				}

				~ShaderEffectImpl()
				{
					context.Destroy(shader);
				}

				Shader* GetShader() override
				{
					return shader;
				}
			private:
				Context& context;

				Shader* shader{ nullptr };
			};


			class ShaderEffectSystemImpl final : public ShaderEffectSystem
			{
			public:
				ShaderEffectSystemImpl(const ShaderEffectSystemCreateInfo& create_info) :
					context{ *create_info.context },
					fs{ *create_info.filesystem_wrapper }
				{
					// Create shader compiler
					ShaderEffectCompilerCreateInfo shader_compiler_ci{};
					shader_compiler_ci.filesystem_wrapper = create_info.filesystem_wrapper;
					shader_compiler_ci.shader_compile_config_path = create_info.effect_database_compile_config_path;
					shader_compiler_ci.shader_cache_patch = create_info.effect_database_path;

					effect_compiler = std::make_unique<ShaderEffectCompiler>(shader_compiler_ci);
				}

				~ShaderEffectSystemImpl()
				{

				}

				ShaderEffect& LoadEffect(std::string_view effect_name) override
				{
					static std::unique_ptr<ShaderEffect> empty_effect = std::make_unique<ShaderEffectImpl>(context, ShaderEffectCreateInfo{});

					// At first try to find effect, may be it's already loaded
					auto effect_key = GetUniqueHashName(effect_name);
					auto found_shader = effect_cache.find(effect_key);
					if (found_shader != effect_cache.end())
						return *found_shader->second;

					auto effect_create_info = effect_compiler->GetEffect(effect_name);
					if (effect_create_info.shader_stages.empty())
						return *empty_effect;

					auto new_effect = effect_cache.emplace(effect_key, std::make_unique<ShaderEffectImpl>(context, effect_create_info));
					return *(new_effect.first->second);
				}

				Context& GetContext() override
				{
					return context;
				}

				Context const& GetContext() const override
				{
					return context;
				}

			private:
				Context& context;
				FilesystemWrapper& fs;

				std::unique_ptr<ShaderEffectCompiler> effect_compiler;

				std::unordered_map<uint32_t, Sampler*> sampler_cache{};
				std::unordered_map<uint32_t, std::unique_ptr<ShaderEffect>> effect_cache{};
			};


			std::unique_ptr<ShaderEffectSystem> CreateShaderEffectSystem(const ShaderEffectSystemCreateInfo& create_info)
			{
				return std::make_unique<ShaderEffectSystemImpl>(create_info);
			}
		}
	}
}