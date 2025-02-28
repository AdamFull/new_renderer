#ifndef SHLIB_DEBUG_TEXT_H
#define SHLIB_DEBUG_TEXT_H

#include "ptr.h"

SHLIB_NS_BEGIN

struct DebugTextData
{
	uint32_t word;
	i16vec2 pos;
	float scale;
	u8vec4 fg_color;
	u8vec4 bg_color;
};

REGISTER_TYPE_PTR(DebugTextData, 4);

struct DebugTextStyle
{
	u8vec4 fg_color;
	u8vec4 bg_color;
};

REGISTER_TYPE_PTR(DebugTextStyle, 1);

SHLIB_NS_END

#endif // SHLIB_DEBUG_TEXT_H