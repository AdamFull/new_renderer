#pragma once

#include "EditorWindow.h"

namespace engine
{
	class PostProcessWindow : public IEditorWindow
	{
	public:
		PostProcessWindow(const std::string& sname) { m_name = sname; }
		virtual ~PostProcessWindow() override = default;

		void Create() override;
	private:
		void __draw(float delta_time) override;
	};
}