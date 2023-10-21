#pragma once

#include <interface/RenderInterface.h>

class asIScriptEngine;

namespace engine
{
	namespace render
	{
		class CRenderConfig
		{
		public:
			void initialize(IRenderInterface* render_interface, IRenderHandleInterface* render_context);

			void execute(const std::vector<uint8_t>& script);
		private:
			asIScriptEngine* scriptEngine{ nullptr };
		};
	}
}