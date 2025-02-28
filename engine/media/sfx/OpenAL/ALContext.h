#pragma once

#include <Context.h>

struct ALCdevice;
struct ALCcontext;

namespace engine
{
	namespace sfx
	{
		namespace openal
		{
			class OpenALContext final : public IContext
			{
			public:
				OpenALContext();
				~OpenALContext();

				void Destroy(SFXResource* handle) override;

				Buffer* CreateBuffer(const BufferCreateInfo& create_info) override;

				Source* CreateSource() override;

				Listener* CreateListener() override;
			private:
				ALCdevice* device{ nullptr };
				ALCcontext* context{ nullptr };
				int8_t current_context{};
			};
		}
	}
}