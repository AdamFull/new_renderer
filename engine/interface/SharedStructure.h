#pragma once

#include <utils/hash.h>

namespace engine
{
	//namespace
	//{
		using namespace utl::utils;
		enum class EMimeType : u32
		{
			eJPG = ".jpg"_utl_hash,
			eJPEG = ".jpeg"_utl_hash,
			ePNG = ".png"_utl_hash,
			eTGA = ".tga"_utl_hash,
			eBMP = ".bmp"_utl_hash,
			ePSD = ".psd"_utl_hash,
			eGIF = ".gif"_utl_hash,
			eHDR = ".hdr"_utl_hash,
			ePIC = ".pic"_utl_hash,
			ePNM = ".pnm"_utl_hash,
			eKTX = ".ktx"_utl_hash,
			eKTX2 = ".ktx2"_utl_hash,
			eDDS = ".dds"_utl_hash,
		};
	//}
}