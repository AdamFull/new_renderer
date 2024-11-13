#include "VorbisDecoder.h"

#include <vorbis/codec.h>

namespace engine
{
	namespace media
	{
		VorbisDecoder::VorbisDecoder()
		{
			vorbis_info_init(vi);
			vorbis_comment_init(vc);
		}

		VorbisDecoder::~VorbisDecoder()
		{
			vorbis_comment_clear(vc);
			vorbis_info_clear(vi);
		}
	}
}