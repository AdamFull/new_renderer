#pragma once

#include "Component.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FBaseComponent
			{
				DECLARE_COMPONENT(FBaseComponent, icon::entity, "Base")

				FBaseComponent() = default;
				FBaseComponent(std::string_view name) : name(name) {}
				std::string name{};
				bool enabled{ true };
				bool visible{ true };
			};
		}
	}
}