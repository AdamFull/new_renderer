#pragma once

#include "../../gfx_common.h"
#include "../../gfx_structures.h"

namespace edge
{
	namespace gfx
	{
		class Context;

		namespace ext
		{
			struct ShaderEffectSystemCreateInfo
			{
				Context* context{ nullptr };
				FilesystemWrapper* filesystem_wrapper{ nullptr };

				std::string_view effect_database_compile_config_path{ "" };
				std::string_view effect_database_path{ "" };
			};

			class ShaderEffectSystem
			{
			public:
				virtual ~ShaderEffectSystem() = default;

				virtual ShaderEffect& LoadEffect(std::string_view effect_name) = 0;

				virtual Context& GetContext() = 0;
				virtual Context const& GetContext() const = 0;
			};

			std::unique_ptr<ShaderEffectSystem> CreateShaderEffectSystem(const ShaderEffectSystemCreateInfo& create_info);
		}
	}
}