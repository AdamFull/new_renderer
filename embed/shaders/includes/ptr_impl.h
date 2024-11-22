#ifndef SHLIB_PTR_IMPL_H
#define SHLIB_PTR_IMPL_H

#include "ptr_common.h"

#define REGISTER_ARRAY_PTR(DataType, Alignment) typedef uint64_t GET_ARR_PTR_T(DataType)
#define REGISTER_TYPE_PTR(DataType, Alignment) typedef uint64_t GET_PTR_T(DataType)

#endif