#pragma once

#include <engine/interface/EngineVariable.h>

namespace engine
{
	class CBoolVariable : public IVariable
	{
	public:
		CBoolVariable(const FNumericScalarInitialData<bool>& initial) : m_value{ initial.value } {}
		~CBoolVariable() = default;

		void display() override;
	private:
		bool m_value{ false };
	};

	class CIntVariable : public IVariable
	{
	public:
		CIntVariable(const FNumericScalarInitialData<i32>& initial) : m_value{ initial.value } {}
		~CIntVariable() = default;

		void display() override;
	private:
		i32 m_value{ 0 };
		i32 m_min{ 0 };
		i32 m_max{ 0 };
	};
}