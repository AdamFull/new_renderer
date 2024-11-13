#pragma once

#include <engine/media/DecoderInterface.h>

struct vorbis_info;
struct vorbis_comment;

namespace engine
{
	namespace media
	{
		class VorbisDecoder final : public IMediaDecoder
		{
		public:
			VorbisDecoder();
			~VorbisDecoder();
		private:
			vorbis_info* vi{ nullptr };
			vorbis_comment* vc{ nullptr };
		};
	}
}