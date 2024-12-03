#pragma once

#include "gfx_enum.h"
#include "gfx_format.h"

namespace engine
{
	namespace gfx
	{
		template<typename _Ty>
		_Ty from_string(const std::string& str) {};

		// Format
		std::string to_string(const Format& format);
		template<> Format from_string<Format>(const std::string& str);

		// ShaderStageFlag
		std::string to_string(const ShaderStageFlag& flag);
		template<> ShaderStageFlag from_string<ShaderStageFlag>(const std::string& str);
	}
}