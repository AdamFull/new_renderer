#pragma once

#include "interface/EngineVariable.h"

namespace engine
{
	class CBoolVariable : public IVariable
	{
	public:
		CBoolVariable(const FNumericScalarInitialData<bool>& initial) : m_value{ initial.value } {}
		~CBoolVariable() = default;

		void Display() override;
	private:
		bool m_value{ false };
	};

	class CIntVariable : public IVariable
	{
	public:
		CIntVariable(const FNumericScalarInitialData<int32_t>& initial) : m_value{ initial.value } {}
		~CIntVariable() = default;

		void Display() override;
	private:
		int32_t m_value{ 0 };
		int32_t m_min{ 0 };
		int32_t m_max{ 0 };
	};
}