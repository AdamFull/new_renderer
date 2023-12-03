#include "ScriptEngine.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

using namespace engine::scripting;

void CScriptEngine::initialize()
{
	//mono_set_assemblies_path("./mono/lib/4.5");

	//auto m_pRootDomain = mono_jit_init("IncenerateEngine");
}