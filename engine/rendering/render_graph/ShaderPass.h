#pragma once

#include <engine/rendering/ExecutionContext.h>
#include <engine/rendering/gfx/Base.h>

namespace engine
{
	namespace gfx
	{
		class Context;
	}

	namespace render
	{
		class ShaderPass
		{
		public:
			ShaderPass(gfx::Context& context);
			virtual ~ShaderPass();

			void SetExecutionContext(ExecutionContext* exec_context);

			void Execute(float delta_time);
		protected:
			virtual void _Execute(gfx::CommandList& command_list, float delta_time) = 0;
		protected:
			gfx::Context& context;

			std::string pass_label{};

			ExecutionContext* execution_context{ nullptr };

			gfx::Shader* shader{ nullptr };
			gfx::RootSignature* root_signature{ nullptr };
			gfx::Pipeline* pipeline{ nullptr };
		};
	}
}