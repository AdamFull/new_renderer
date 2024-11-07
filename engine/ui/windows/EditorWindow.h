#pragma once

#include <standart_library.h>

namespace engine
{
	class IEditorWindow
	{
	public:
		virtual ~IEditorWindow() {}

		virtual void Create() = 0;
		virtual void Draw(float fDt);

		const std::string& GetName() const { return m_name; }
		const bool IsOpen() const { return m_bIsOpen; }
		void ToggleEnable() { m_bIsOpen = !m_bIsOpen; }
	protected:
		virtual void __draw(float fDt) = 0;

		std::string m_name;
		bool m_bIsOpen{ true };
	};
}