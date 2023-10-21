#ifndef SCRIPTSTDVECTOR_H
#define SCRIPTSTDVECTOR_H

#ifndef ANGELSCRIPT_H 
// Avoid having to inform include path if header is already include before
#include <angelscript.h>
#endif

#include <aghelpers.h>

#include <vector>
#include <format>
#include <cassert>

template<class _Ty>
inline _Ty& VectorIndex(size_t index, std::vector<_Ty>& vec)
{
	if (index < vec.size())
		return vec[index];

	asIScriptContext* ctx = asGetActiveContext();
	ctx->SetException("Vector index out of range");
	static _Ty dummy;
	return dummy;
}

template<class _Ty>
void RegisterStdVector(asIScriptEngine* engine)
{
	using _VectorType = std::vector<_Ty>;
	auto type_name = GetASTypeName<_Ty>();
	auto vector_type_name = std::format("vector_{}", type_name);

	int32_t res = engine->RegisterObjectType(vector_type_name.c_str(), sizeof(_VectorType), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);
	assert(res >= 0);

	// constructor
    res = engine->RegisterObjectBehaviour(vector_type_name.c_str(), asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(DefaultTypeConstructor<_VectorType>), asCALL_CDECL_OBJLAST);
    assert(res >= 0);

	// destructor
	res  = engine->RegisterObjectBehaviour(vector_type_name.c_str(), asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DefaultTypeDestructor<_VectorType>), asCALL_CDECL_OBJLAST);
    assert(res >= 0);

	auto copy_construct_signature = std::format("void f(const {} &in)", vector_type_name);
	res = engine->RegisterObjectBehaviour(vector_type_name.c_str(), asBEHAVE_CONSTRUCT, copy_construct_signature.c_str(), asFUNCTION(DefaultTypeCopyConstruct<_VectorType>), asCALL_CDECL_OBJLAST);
	assert(res >= 0);

	auto copy_operator_signature = std::format("{0}& opAssign(const {0} &in)", vector_type_name);
	res = engine->RegisterObjectMethod(vector_type_name.c_str(), copy_operator_signature.c_str(), asMETHODPR(_VectorType, operator=, (const _VectorType&), _VectorType&), asCALL_THISCALL);
	assert(res >= 0);

	auto at_signature = std::format("{}& at(uint64)", type_name);
	res = engine->RegisterObjectMethod(vector_type_name.c_str(), at_signature.c_str(), asMETHODPR(_VectorType, at, (size_t), _Ty&), asCALL_THISCALL);
	assert(res >= 0);

	auto push_back_signature = std::format("void push_back(const {} &in)", type_name);
	res = engine->RegisterObjectMethod(vector_type_name.c_str(), push_back_signature.c_str(), asMETHODPR(_VectorType, push_back, (const _Ty&), void), asCALL_THISCALL);
	assert(res >= 0);

	auto index_operator_signature = std::format("{}& opIndex(uint64)", type_name);
	res = engine->RegisterObjectMethod(vector_type_name.c_str(), index_operator_signature.c_str(), asFUNCTION(VectorIndex<_Ty>), asCALL_CDECL_OBJLAST);
	assert(res >= 0);
}

#endif