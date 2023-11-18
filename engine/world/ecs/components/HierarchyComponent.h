#pragma once

#include <vector>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FHierarchyComponent
			{
				uint32_t parent{ ~0u };
				std::vector<uint32_t> children{};
			};
		}
	}
}