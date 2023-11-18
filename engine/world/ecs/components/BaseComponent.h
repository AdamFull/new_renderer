#pragma once

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
			};
		}
	}
}