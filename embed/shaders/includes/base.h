#ifndef SHLIB_BASE_H
#define SHLIB_BASE_H

#define INVALID_HANDLE ~0u

#ifdef __cplusplus

#include <cstdint>
#include <array>
#include <glm/glm.hpp>

#define SHLIB_NS_BEGIN namespace shlib {
#define SHLIB_NS_END }

#define ALIGN_AS(x) alignas(x)
#define ARRAY_OF(name, type, count) std::array<type, count> name

#define CONSTANT constexpr const

#define CB_BEGIN(BufferName, BufferSet, BufferBinding) struct BufferName {
#define CB_END(BufferName) };
#define CB_END_E() };

SHLIB_NS_BEGIN
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat2 = glm::mat2;
using mat3 = glm::mat3;
using mat4 = glm::mat4;
SHLIB_NS_END

#else // __glslang

#define SHLIB_NS_BEGIN
#define SHLIB_NS_END

#define ALIGN_AS(x)
#define ARRAY_OF(name, type, count) type name[count]

#define CONSTANT const

#define CB_BEGIN(BufferName, BufferSet, BufferBinding) layout(std140, set = BufferSet, binding = BufferBinding) uniform BufferName {
#define CB_END(BufferName) } BufferName;
#define CB_END_E() };

#define REGISTER_RW_STORAGE_BUFFER(Set, Binding, BufferTypeName, DataType, Name) layout(std430, set = Set, binding = Binding) buffer BufferTypeName { DataType data[]; } Name;
#define REGISTER_RO_STORAGE_BUFFER(Set, Binding, BufferTypeName, DataType, Name) layout(std430, set = Set, binding = Binding) readonly buffer BufferTypeName { DataType data[]; } Name;
#define REGISTER_WO_STORAGE_BUFFER(Set, Binding, BufferTypeName, DataType, Name) layout(std430, set = Set, binding = Binding) writeonly buffer BufferTypeName { DataType data[]; } Name;

#endif // __cplusplus

#endif // SHLIB_BASE_H