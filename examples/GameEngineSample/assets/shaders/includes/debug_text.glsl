#ifndef SHLIB_DEBUG_TEXT_GLSL
#define SHLIB_DEBUG_TEXT_GLSL

#include "debug_text.h"

layout(push_constant) uniform DebugTextDrawData
{
	GET_PTR_T(DebugTextData) word_data;
	vec2 inv_viewport_resolution;
} draw_data;

#endif // SHLIB_DEBUG_TEXT_GLSL