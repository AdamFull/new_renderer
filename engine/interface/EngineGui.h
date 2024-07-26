#pragma once

#include <reactive/function.h>
#include <standart_library.h>

namespace engine
{
	class IUIElement
	{
	public:
		virtual ~IUIElement() = default;

		virtual void OnInit() = 0;
		virtual void PushElement(IUIElement* element) = 0;
		virtual void OnGui(f32 dt) = 0;

	protected:
		std::string m_label;
	};
}