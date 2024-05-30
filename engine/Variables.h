#pragma once

#include <engine/interface/EngineVariable.h>

namespace engine
{
	class CBoolVariable : public IVariable
	{
	public:
		CBoolVariable(bool val) : m_value{ val } {}
		~CBoolVariable() = default;

		void display() override;
	private:
		bool m_value{ false };
	};
}