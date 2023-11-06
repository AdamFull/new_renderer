#include <render_config/RenderConfig.h>
#include <render_config/RenderConfigBindings.h>

#include <sol/sol.hpp>

#include <logger/logger.h>

using namespace engine::render;

template<class _Ty>
void safe_delete(_Ty*& ptr)
{
	if (ptr != nullptr)
	{
		delete ptr;
		ptr = nullptr;
	}
}

CRenderScriptExecutor::~CRenderScriptExecutor()
{
	safe_delete(m_pEnvironment);

	for (auto& [id, function] : m_scriptFunctions)
		safe_delete(function);
	m_scriptFunctions.clear();
}

int CRenderScriptExecutor::run(const char* function)
{
	auto found_function = m_scriptFunctions.find(utl::fnv1a_64_hash(function));
	if (found_function != m_scriptFunctions.end())
	{
		//m_pContext->Unprepare();
		//m_pContext->Prepare(found_function->second);
		//return m_pContext->Execute();
		return 0;
	}

	return -1;
}

CRenderScriptCore::~CRenderScriptCore()
{
	safe_delete(m_pLuaState);
}

void CRenderScriptCore::create(IRenderInterface* render_interface, IRenderHandleInterface* render_context)
{
	m_pLuaState = new sol::state();

	m_pLuaState->open_libraries(sol::lib::base, sol::lib::package, sol::lib::math, sol::lib::table, sol::lib::os, sol::lib::string);

	bindGLMath(m_pLuaState);
	bindRenderInterface(m_pLuaState);

	registerEnums();
	registerObjects();
	registerGlobals(render_interface, render_context);
}

std::unique_ptr<CRenderScriptExecutor> CRenderScriptCore::build(const std::string& name, const std::vector<uint8_t>& program_code)
{
	auto render_script = std::make_unique<CRenderScriptExecutor>();
	render_script->m_pEnvironment = new sol::environment(*m_pLuaState, sol::create, m_pLuaState->globals());

	std::string script_code{ program_code.begin(), program_code.end() };
	auto result = m_pLuaState->safe_script(script_code, *render_script->m_pEnvironment, sol::script_pass_on_error);
	if (!result.valid())
	{
		sol::error err = result;
		log_error("Script loading failed: {}", err.what());
		return nullptr;
	}

	std::vector<std::string> function_names{};
	render_script->m_pEnvironment->for_each([&function_names](sol::object const& key, sol::object const& value) { if (value.is<sol::function>()) function_names.emplace_back(key.as<std::string>()); });

	for (const auto& func : function_names)
		render_script->m_scriptFunctions[utl::fnv1a_64_hash(func.c_str())] = new sol::protected_function(render_script->m_pEnvironment->get<sol::protected_function>(func));

	m_pLuaState->collect_garbage();

	return render_script;
}

void CRenderScriptCore::registerEnums()
{
	
}

void CRenderScriptCore::registerObjects()
{
}

void CRenderScriptCore::registerGlobals(IRenderInterface* render_interface, IRenderHandleInterface* render_context)
{
	(*m_pLuaState)["render_interface"] = render_interface;
	(*m_pLuaState)["render_context"] = render_context;
}