#pragma once

#include <vector>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FMeshPrimitive
			{
				struct MetaData
				{

				} meta;

				size_t material{};
				bool castShadows{ false };
			};

			struct FMeshComponent
			{
				std::vector<FMeshPrimitive> primitives{};
			};
		}
	}
}