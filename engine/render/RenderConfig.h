#pragma once

#include <interface/RenderInterface.h>

class asIScriptEngine;
class asIScriptModule;
class asIScriptFunction;
class asIScriptContext;

namespace engine
{
	namespace render
	{
		class CRenderScriptExecutor
		{
		public:
			friend class CRenderScriptCore;

			~CRenderScriptExecutor();

			int run(size_t function);
		private:
			asIScriptModule* m_pScriptModule{ nullptr };
			std::unordered_map<size_t, asIScriptFunction*> m_scriptFunctions{};
			asIScriptContext* m_pContext{ nullptr };
		};

		class CRenderScriptCore
		{
		public:
			void create(IRenderInterface* render_interface, IRenderHandleInterface* render_context);
			void build(const std::unordered_map<std::string, std::vector<uint8_t>>& programs);

			int execute(const char* script_name, const char* function = "void main()");
		protected:
			void registerEnums();
			void registerObjects();
			void registerGlobals(IRenderInterface* render_interface, IRenderHandleInterface* render_context);
		private:
			asIScriptEngine* m_pScriptEngine{ nullptr };
			std::unordered_map<size_t, std::unique_ptr<CRenderScriptExecutor>> m_scriptExecutors{};
		};

		class CRenderStage
		{
		public:
		private:
			asIScriptModule* scriptModule{ nullptr };
		};
	}
}