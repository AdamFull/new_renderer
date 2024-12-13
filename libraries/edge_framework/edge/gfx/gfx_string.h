#pragma once

#include "gfx_enum.h"
#include "gfx_format.h"

namespace edge
{
	namespace gfx
	{
		template<typename _Ty>
		_Ty from_string(const std::string& str) {};

		// Format
		std::string to_string(const Format& format);
		template<> Format from_string<Format>(const std::string& str);
	}
}