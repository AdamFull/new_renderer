#pragma once

#include <hash.hpp>

namespace engine
{
	//namespace
	//{
		using namespace utl;
		enum class EMimeType : uint32_t
		{
			eJPG = ".jpg"_hash32,
			eJPEG = ".jpeg"_hash32,
			ePNG = ".png"_hash32,
			eTGA = ".tga"_hash32,
			eBMP = ".bmp"_hash32,
			ePSD = ".psd"_hash32,
			eGIF = ".gif"_hash32,
			eHDR = ".hdr"_hash32,
			ePIC = ".pic"_hash32,
			ePNM = ".pnm"_hash32,
			eKTX = ".ktx"_hash32,
			eKTX2 = ".ktx2"_hash32,
			eDDS = ".dds"_hash32,
		};
	//}
}