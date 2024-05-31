#pragma once

#include <engine/interface/EngineVariable.h>

namespace engine
{
	class CVariableManager : public IVariableManager
	{
	public:
		~CVariableManager() = default;

		std::unique_ptr<IVariable> makeVariable(const char* name, const FVariableCreateInfo& create_info) override;
		std::unique_ptr<IVariableBlock> makeVariableBlock(const char* name) override;
	};
}