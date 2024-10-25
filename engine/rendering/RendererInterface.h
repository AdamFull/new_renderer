#pragma once

#include <engine/rendering/gfx/Context.h>
#include <engine/rendering/gfx/ResourceLoader.h>

namespace engine
{
	namespace render
	{
		class IRenderer
		{
		public:
			virtual ~IRenderer() = default;

			virtual void Render(float delta_time) = 0;
		};
	}
}