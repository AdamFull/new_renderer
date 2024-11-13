#pragma once

#include "MediaEngineInterface.h"

namespace engine
{
	namespace media
	{
		class MediaEngine final : IMediaEngine
		{
		public:
			MediaEngine();
			~MediaEngine() = default;
		};
	}
}