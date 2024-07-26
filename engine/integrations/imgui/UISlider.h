#pragma once

#include <reactive/function.h>
#include <engine/interface/EngineGui.h>

namespace engine
{
	template<typename _Ty, size_t _Count>
	class CUISlider : public IUIElement
	{
	public:
	protected:
		std::string m_label{};
		_Ty& m_value;
	};
}