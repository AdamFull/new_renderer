#ifndef SHLIB_CULLING_H
#define SHLIB_CULLING_H

#include "draw.h"

SHLIB_NS_BEGIN

CB_BEGIN(CullingInfo, 1, 0)
	GET_PTR_T(DrawDescription) source_descriptions;
	GET_PTR_T(DrawDescription) alive_descriptions;
	GET_PTR_T(DrawIndexedIndirectData) alive_draw_commands;
	GET_PTR_T(uint32_t) alive_bits;
	GET_PTR_T(uint32_t) draw_counts;
CB_END_E()

SHLIB_NS_END

#endif // SHLIB_CULLING_H