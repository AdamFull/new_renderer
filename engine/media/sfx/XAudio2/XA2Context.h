#pragma once 

#include <Context.h>

struct IXAudio2;
struct IXAudio2MasteringVoice;

namespace engine
{
	namespace sfx
	{
		namespace xa2
		{
			class XAudio2Context final : public IContext
			{
			public:
				XAudio2Context();
				~XAudio2Context();

				void Destroy(SFXResource* handle) override;

				Buffer* CreateBuffer(const BufferCreateInfo& create_info) override;

				Source* CreateSource() override;

				Listener* CreateListener() override;
			private:
				IXAudio2* context{ nullptr };
				IXAudio2MasteringVoice* mastering_voice{ nullptr };
			};
		}
	}
}