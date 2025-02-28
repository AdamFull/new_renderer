#pragma once

#include "Base.h"

namespace engine
{
	namespace sfx
	{
		class IContext
		{
		public:
			virtual ~IContext() = default;

			virtual void Destroy(SFXResource* handle) = 0;

			virtual [[nodiscard]] Buffer* CreateBuffer(const BufferCreateInfo& create_info) = 0;

			virtual [[nodiscard]] Source* CreateSource() = 0;

			virtual [[nodiscard]] Listener* CreateListener() = 0;
		};

		std::unique_ptr<IContext> CreateContext(const ContextCreateInfo& create_info);
	}
}