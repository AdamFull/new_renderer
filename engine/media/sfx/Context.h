#pragma once

#include "Base.h"

namespace engine
{
	namespace sfx
	{
		class Context
		{
		public:
			virtual ~Context() = default;

			virtual void Destroy(SFXResource* handle) = 0;

			virtual Buffer* CreateBuffer(const BufferCreateInfo& create_info) = 0;

			virtual Source* CreateSource() = 0;

			virtual Listener* CreateListener() = 0;
		};

		std::unique_ptr<Context> CreateContext(const ContextCreateInfo& create_info);
	}
}