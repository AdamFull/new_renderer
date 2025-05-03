#ifndef SHLIB_PTR_IMPL_GLSL
#define SHLIB_PTR_IMPL_GLSL

#include "ptr_common.h"

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require

#define REGISTER_ARRAY_PTR(DataType, Alignment) layout(std430, buffer_reference, buffer_reference_align = Alignment) buffer GET_ARR_PTR_T(DataType) { DataType data[]; };
#define REGISTER_TYPE_PTR(DataType, Alignment) layout(std430, buffer_reference, buffer_reference_align = Alignment) buffer GET_PTR_T(DataType) { DataType data; };

#endif