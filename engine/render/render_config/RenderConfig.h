#pragma once

#include <interface/RenderInterface.h>
//#include <ShaderObject.h>

#include <sol/forward.hpp>

namespace engine
{
	namespace render
	{
		//class CDynamicRenderStageScript
		//{
		//public:
		//	void create();
		//	void update(float dt);
		//private:
		//	std::unique_ptr<CRenderScriptExecutor> m_pScriptExecutor{};
		//	std::unique_ptr<CShaderObject> m_pStageShader{};
		//};

		class CRenderScriptExecutor
		{
		public:
			friend class CRenderScriptCore;

			~CRenderScriptExecutor();

			int run(const char* function = "void main()");
		private:
			sol::environment* m_pEnvironment{ nullptr };
			std::unordered_map<size_t, sol::protected_function*> m_scriptFunctions{};
		};

		class CRenderScriptCore
		{
		public:
			~CRenderScriptCore();
			void create(IRenderInterface* render_interface, IRenderHandleInterface* render_context);
			std::unique_ptr<CRenderScriptExecutor> build(const std::string& name, const std::vector<uint8_t>& program_code);
		protected:
			void registerEnums();
			void registerObjects();
			void registerGlobals(IRenderInterface* render_interface, IRenderHandleInterface* render_context);
		private:
			sol::state* m_pLuaState{ nullptr };
		};
	}
}