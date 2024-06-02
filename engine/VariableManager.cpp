#include "VariableManager.h"

#include "VariableBlock.h"
#include "Variables.h"

using namespace engine;

void CVariableManager::display()
{

}


std::unique_ptr<IVariable> CVariableManager::makeVariable(const char* name, const FVariableCreateInfo& create_info)
{
	std::unique_ptr<IVariable> variable{};

	if (create_info.type == EVariableType::eScalar)
	{
		switch (create_info.dtype)
		{
		case EVariableDataType::eBool: {
			auto initial = static_cast<FNumericScalarInitialData<bool>*>(create_info.initial);
			return std::make_unique<CBoolVariable>(*initial);
		} break;
		case EVariableDataType::eInt: {
			auto initial = static_cast<FNumericScalarInitialData<i32>*>(create_info.initial);
		} break;
		case EVariableDataType::eUInt: {
			auto initial = static_cast<FNumericScalarInitialData<u32>*>(create_info.initial);
		} break;
		case EVariableDataType::eInt64: {
			auto initial = static_cast<FNumericScalarInitialData<i64>*>(create_info.initial);
		} break;
		case EVariableDataType::eUInt64: {
			auto initial = static_cast<FNumericScalarInitialData<u64>*>(create_info.initial);
		} break;
		case EVariableDataType::eFloat: {
			auto initial = static_cast<FNumericScalarInitialData<f32>*>(create_info.initial);
		} break;
		case EVariableDataType::eDouble: {
			auto initial = static_cast<FNumericScalarInitialData<f64>*>(create_info.initial);
		} break;
		}
	}
	else if (create_info.type == EVariableType::eVector)
	{
		switch (create_info.dtype)
		{
		case EVariableDataType::eBool: {
			auto initial = static_cast<FNumericVectorInitialData<bool>*>(create_info.initial);
		} break;
		case EVariableDataType::eInt: {
			auto initial = static_cast<FNumericVectorInitialData<i32>*>(create_info.initial);
		} break;
		case EVariableDataType::eUInt: {
			auto initial = static_cast<FNumericVectorInitialData<u32>*>(create_info.initial);
		} break;
		case EVariableDataType::eInt64: {
			auto initial = static_cast<FNumericVectorInitialData<i64>*>(create_info.initial);
		} break;
		case EVariableDataType::eUInt64: {
			auto initial = static_cast<FNumericVectorInitialData<u64>*>(create_info.initial);
		} break;
		case EVariableDataType::eFloat: {
			auto initial = static_cast<FNumericVectorInitialData<f32>*>(create_info.initial);
		} break;
		case EVariableDataType::eDouble: {
			auto initial = static_cast<FNumericVectorInitialData<f64>*>(create_info.initial);
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