#ifndef SHLIB_BASE_H
#define SHLIB_BASE_H

#define INVALID_HANDLE ~0u

#ifdef __cplusplus

#include <cstdint>
#include <array>
#include <hash.hpp>

#include <glm/glm.hpp>
#include <glm/packing.hpp>

#define SHLIB_INLINE inline

#define SHLIB_NS_BEGIN namespace shlib {
#define SHLIB_NS_END }

#define ALIGN_AS(x) alignas(x)
#define ARRAY_OF(name, type, count) std::array<type, count> name

#define CONSTANT constexpr const

#define CB_BEGIN(BufferName, BufferSet, BufferBinding) \
constexpr const uint32_t BufferName##_label = utl::murmur_hash(#BufferName); \
struct BufferName {
#define CB_END(BufferName) };
#define CB_END_E() };

SHLIB_NS_BEGIN
using namespace glm;
SHLIB_NS_END

#else

#define SHLIB_INLINE

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


#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_EXT_shader_8bit_storage : enable
#extension GL_EXT_shader_16bit_storage : enable
#extension GL_EXT_control_flow_attributes : enable

#extension GL_EXT_shader_explicit_arithmetic_types : require
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require

#endif // __cplusplus

#endif // SHLIB_BASE_H