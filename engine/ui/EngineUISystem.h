#pragma once

#include <engine/runtime/InputEvents.h>
#include <engine/ui/windows/EditorWindow.h>

namespace engine
{
	class EngineUISystem
	{
	public:
		EngineUISystem();
		~EngineUISystem() = default;

		void Initialize();

		void Update(f32 dt);

		void Destroy();
	private:
		utl::vector<std::unique_ptr<IEditorWindow>> m_vWindows{};
	};
}