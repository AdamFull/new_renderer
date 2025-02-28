#pragma once

#include "../../gfx_common.h"
#include "../../gfx_structures.h"

namespace edge
{
	namespace gfx
	{
		class IContext;

		namespace ext
		{
			struct ShaderEffectSystemCreateInfo
			{
				IContext* context{ nullptr };
				FilesystemWrapper* filesystem_wrapper{ nullptr };

				std::string_view effect_database_compile_config_path{ "" };
				std::string_view effect_database_path{ "" };
			};

			class ShaderEffectSystem
			{
			public:
				virtual ~ShaderEffectSystem() = default;

				virtual IShaderEffect& LoadEffect(std::string_view effect_name) = 0;

				virtual IContext& GetContext() = 0;
				virtual IContext const& GetContext() const = 0;
			};

			std::unique_ptr<ShaderEffectSystem> CreateShaderEffectSystem(const ShaderEffectSystemCreateInfo& create_info);
		}
	}
}