#ifndef SCRIPTASHELPERS_H
#define SCRIPTASHELPERS_H

#ifndef ANGELSCRIPT_H 
// Avoid having to inform include path if header is already include before
#include <angelscript.h>
#endif

#include <string>
#include <format>

template<typename _Ty> inline std::string GetASTypeName();
template<> inline std::string GetASTypeName<int8_t>() { return "int8"; }
template<> inline std::string GetASTypeName<int16_t>() { return "int16"; }
template<> inline std::string GetASTypeName<int32_t>() { return "int"; }
template<> inline std::string GetASTypeName<int64_t>() { return "int64"; }
template<> inline std::string GetASTypeName<uint8_t>() { return "uint8"; }
template<> inline std::string GetASTypeName<uint16_t>() { return "uint16"; }
template<> inline std::string GetASTypeName<uint32_t>() { return "uint"; }
template<> inline std::string GetASTypeName<uint64_t>() { return "uint64"; }

template<typename _Ty>
inline std::string GetASTypeName()
{
	std::string typeName = typeid(_Ty).name();

	std::size_t pos;
	while ((pos = typeName.find("class ")) != std::string::npos)
		typeName.erase(pos, 6);

	while ((pos = typeName.find("struct ")) != std::string::npos)
		typeName.erase(pos, 7);

	while ((pos = typeName.find("::")) != std::string::npos)
		typeName = typeName.substr(pos + 2);

	return typeName;
}

template<class _Ty>
inline void DefaultTypeConstructor(void* memory)
{
	new(memory) _Ty();
}

template<class _Ty>
inline void DefaultTypeDestructor(void* memory)
{
	((_Ty*)memory)->~_Ty();
}

template<class _Ty>
inline void DefaultTypeCopyConstruct(const _Ty& other, void* memory)
{
	new(memory) _Ty(other);
}

template<class _Ty>
inline _Ty& DefaultTypeCopyAssignment(const _Ty& other, _Ty* memory)
{
	*memory = other;
	return *memory;
}

template<class _Ty>
inline void RegisterASType(asIScriptEngine* engine, const std::string& type_name)
{
	auto copy_constructor_signature = std::format("void f(const {} &in)", type_name);
	auto copy_assignment_operator_signature = std::format("{0} &opAssign(const {0} &in)", type_name);

	engine->RegisterObjectType(type_name.c_str(), sizeof(_Ty), asOBJ_VALUE | asGetTypeTraits<_Ty>());
	engine->RegisterObjectBehaviour(type_name.c_str(), asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(DefaultTypeConstructor<_Ty>), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectBehaviour(type_name.c_str(), asBEHAVE_CONSTRUCT, copy_constructor_signature.c_str(), asFUNCTION(DefaultTypeCopyConstruct<_Ty>), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod(type_name.c_str(), copy_assignment_operator_signature.c_str(), asFUNCTION(DefaultTypeCopyAssignment<_Ty>), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectBehaviour(type_name.c_str(), asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DefaultTypeDestructor<_Ty>), asCALL_CDECL_OBJLAST);
}

template<class _Ty>
inline void RegisterASType(asIScriptEngine* engine)
{
	auto type_name = GetASTypeName<_Ty>();
	RegisterASType<_Ty>(engine, type_name);
}

#endif