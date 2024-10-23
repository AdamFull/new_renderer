#pragma once

#include <GFXBase.h>

namespace engine
{
	namespace gfx
	{
		class ExecutionContext
		{
		public:
			virtual ~ExecutionContext() = default;

			virtual CommandList* GetCurrentCommandList() = 0;
		};
	}
}