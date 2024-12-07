#pragma once

#include "interface/EngineVariable.h"

namespace engine
{
	class CVariableBlock : public IVariableBlock
	{
	public:
		~CVariableBlock() = default;

		void Display() override;
	};
}