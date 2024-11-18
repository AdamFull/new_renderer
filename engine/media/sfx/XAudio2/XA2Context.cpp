#include "XA2Context.h"

#include <xaudio2.h>

#include <util/Logger.h>

namespace engine
{
	namespace sfx
	{
		namespace xa2
		{
			XAudio2Context::XAudio2Context()
			{
				if (FAILED(::XAudio2Create(&context, 0, XAUDIO2_DEFAULT_PROCESSOR)))
				{
					log_error("Failed to initialize xaudio2.");
					return;
				}

				if (FAILED(context->CreateMasteringVoice(&mastering_voice)))
				{
					log_error("Failed to initialize xaudio2 mastering voice.");
					return;
				}
			}

			XAudio2Context::~XAudio2Context()
			{
				context->Release();
			}

			void XAudio2Context::Destroy(SFXResource* handle)
			{

			}

			Buffer* XAudio2Context::CreateBuffer(const BufferCreateInfo& create_info)
			{
				return nullptr;
			}

			Source* XAudio2Context::CreateSource()
			{
				return nullptr;
			}

			Listener* XAudio2Context::CreateListener()
			{
				return nullptr;
			}
		}
	}
}