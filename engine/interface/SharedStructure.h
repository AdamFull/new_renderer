#pragma once

#include <hash.hpp>

namespace engine
{
	//namespace
	//{
		using namespace utl;
		enum class EMimeType : u32
		{
			eJPG = ".jpg"_fnv1a32,
			eJPEG = ".jpeg"_fnv1a32,
			ePNG = ".png"_fnv1a32,
			eTGA = ".tga"_fnv1a32,
			eBMP = ".bmp"_fnv1a32,
			ePSD = ".psd"_fnv1a32,
			eGIF = ".gif"_fnv1a32,
			eHDR = ".hdr"_fnv1a32,
			ePIC = ".pic"_fnv1a32,
			ePNM = ".pnm"_fnv1a32,
			eKTX = ".ktx"_fnv1a32,
			eKTX2 = ".ktx2"_fnv1a32,
			eDDS = ".dds"_fnv1a32,
		};
	//}
}