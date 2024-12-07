#pragma once

#include "interface/EngineVariable.h"

namespace engine
{
	class CVariableManager : public IVariableManager
	{
	public:
		~CVariableManager() = default;

		void Display() override;

		std::unique_ptr<IVariable> MakeVariable(const char* name, const FVariableCreateInfo& create_info) override;
		std::unique_ptr<IVariableBlock> MakeVariableBlock(const char* name) override;
	};
}