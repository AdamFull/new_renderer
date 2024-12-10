#pragma once

#include "EditorWindow.h"

namespace engine
{
	class ViewportWindow : public IEditorWindow
	{
	public:
		ViewportWindow(std::string_view sname) { m_name = sname; }
		virtual ~ViewportWindow() override = default;

		void Create() override;
	private:
		void __draw(float fDt) override;

		void DrawViewport();
		void ViewportPicking();
		void DrawManipulator(float offsetx, float offsety, float sizex, float sizey);
		void DrawOverlay(float offsetx, float offsety, float fDt);

		f32 m_fSizeX{};
		f32 m_fSizeY{};
	};
}