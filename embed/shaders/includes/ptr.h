#ifndef SHLIB_PTR_H
#define SHLIB_PTR_H

#include "base.h"

#ifdef __cplusplus
#include "ptr_impl.h"
#else 
#include "ptr_impl.glsl"
#endif // __cplusplus

SHLIB_NS_BEGIN

// Define base pointer types
REGISTER_TYPE_PTR(int8_t, 1);
REGISTER_TYPE_PTR(uint8_t, 1);
REGISTER_TYPE_PTR(int16_t, 2);
REGISTER_TYPE_PTR(uint16_t, 2);
REGISTER_TYPE_PTR(int32_t, 4);
REGISTER_TYPE_PTR(uint32_t, 4);
REGISTER_TYPE_PTR(int64_t, 8);
REGISTER_TYPE_PTR(uint64_t, 8);

REGISTER_TYPE_PTR(u8vec4, 1);

REGISTER_TYPE_PTR(i16vec3, 2);

REGISTER_TYPE_PTR(u16vec2, 2);
REGISTER_TYPE_PTR(u16vec4, 2);

REGISTER_TYPE_PTR(float, 4);

REGISTER_TYPE_PTR(vec2, 4);
REGISTER_TYPE_PTR(vec3, 4);
REGISTER_TYPE_PTR(vec4, 4);

REGISTER_TYPE_PTR(mat2, 4);
REGISTER_TYPE_PTR(mat3, 4);
REGISTER_TYPE_PTR(mat4, 4);

// Define base array pointer types
REGISTER_ARRAY_PTR(int8_t, 1);
REGISTER_ARRAY_PTR(uint8_t, 1);
REGISTER_ARRAY_PTR(int16_t, 2);
REGISTER_ARRAY_PTR(uint16_t, 2);
REGISTER_ARRAY_PTR(int32_t, 4);
REGISTER_ARRAY_PTR(uint32_t, 4);
REGISTER_ARRAY_PTR(int64_t, 8);
REGISTER_ARRAY_PTR(uint64_t, 8);

REGISTER_ARRAY_PTR(float, 4);

REGISTER_ARRAY_PTR(vec2, 8);
REGISTER_ARRAY_PTR(vec3, 8);
REGISTER_ARRAY_PTR(vec4, 16);

REGISTER_ARRAY_PTR(mat2, 16);
REGISTER_ARRAY_PTR(mat3, 16);
REGISTER_ARRAY_PTR(mat4, 64);

#define PTRCAST(pointer_type, address) pointer_type(address)
#define GET_BUFFER_POINTER(pointer_type, buffer_desc) pointer_type(buffer_desc.address)
#define PTRVAL(buffer, index) buffer.data[index]
#define PTRDEREF(buffer, index) buffer[index].data

SHLIB_NS_END

#endif