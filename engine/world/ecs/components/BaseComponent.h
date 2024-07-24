#pragma once

#include <nlohmann/json.hpp>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FBaseComponent
			{
				FBaseComponent() = default;
				FBaseComponent(const std::string& name) : name(name) {}
				std::string name{};
				bool enabled{ true };
				bool visible{ true };

				//NLOHMANN_DEFINE_TYPE_INTRUSIVE(FBaseComponent, name, enabled, visible);
			};
		}
	}
}