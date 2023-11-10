#include <render_config/RenderConfig.h>
#include <render_config/RenderConfigBindings.h>
#include <render/ShaderObject.h>

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

	m_pLuaState->new_usertype<CDynamicRenderStage>(
		"DynamicRenderStage",
		"get_shader", [](CDynamicRenderStage& self) { return self.getShader().get(); },
		"get_render_targets", [](CDynamicRenderStage& self, FGraphicsShaderCreateInfo& shader_ci) {
			auto& rendering_info = self.getRenderingInfo();
			shader_ci.colorAttachments = rendering_info.colorAttachments;
			shader_ci.depthAttachment = rendering_info.depthAttachment;
			shader_ci.stencilAttachment = rendering_info.stencilAttachment;
		}
	);

	(*m_pLuaState)["render_interface"] = render_interface;
	(*m_pLuaState)["render_context"] = render_context;
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

void CDynamicRenderStage::create(const std::string& stage)
{
	m_pStageShader = std::make_unique<CShaderObject>();

	m_pScriptExecutor->run("on_create", stage);
}

void CDynamicRenderStage::update(float dt)
{
	m_pScriptExecutor->run("on_update", dt);
}

const std::unique_ptr<IShaderObject>& CDynamicRenderStage::getShader() const
{
	return m_pStageShader;
}

FBeginRenderingInfo& CDynamicRenderStage::getRenderingInfo()
{
	return m_renderingInfo;
}