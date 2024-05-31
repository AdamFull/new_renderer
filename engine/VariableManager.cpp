#include "VariableManager.h"

#include "VariableBlock.h"
#include "Variables.h"

using namespace engine;


std::unique_ptr<IVariable> CVariableManager::makeVariable(const char* name, const FVariableCreateInfo& create_info)
{
	std::unique_ptr<IVariable> variable{};

	if (create_info.type == EVariableType::eScalar)
	{
		switch (create_info.dtype)
		{
		case EVariableDataType::eBool: {
		} break;
		case EVariableDataType::eInt: {
		} break;
		case EVariableDataType::eUInt: {
		} break;
		case EVariableDataType::eInt64: {
		} break;
		case EVariableDataType::eUInt64: {
		} break;
		case EVariableDataType::eFloat: {
		} break;
		case EVariableDataType::eDouble: {
		} break;
		case EVariableDataType::eString: {
		} break;
		}
	}
	else if (create_info.type == EVariableType::eVector)
	{
		switch (create_info.dtype)
		{
		case EVariableDataType::eBool: {
		} break;
		case EVariableDataType::eInt: {
		} break;
		case EVariableDataType::eUInt: {
		} break;
		case EVariableDataType::eInt64: {
		} break;
		case EVariableDataType::eUInt64: {
		} break;
		case EVariableDataType::eFloat: {
		} break;
		case EVariableDataType::eDouble: {
		} break;
		case EVariableDataType::eString: {
		} break;
		}
	}
	else
	{

	}
}

std::unique_ptr<IVariableBlock> CVariableManager::makeVariableBlock(const char* name)
{
	return std::make_unique<CVariableBlock>();
}