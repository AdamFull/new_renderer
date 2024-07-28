#pragma once

#include "Component.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FBaseComponent : public IComponent
			{
				FBaseComponent() = default;
				FBaseComponent(const std::string& name) : name(name) {}
				std::string name{};
				bool enabled{ true };
				bool visible{ true };
			};
		}
	}
}