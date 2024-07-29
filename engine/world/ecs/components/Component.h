#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <nlohmann/json.hpp>
#include <standart_library.h>

#include <entt/entt.hpp>

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct IComponent
			{
				virtual void Serialize(nlohmann::json& j) {}
				virtual void Deserialize(const nlohmann::json& j) {}

				virtual void OnEdit() {}

				virtual bool IsSerializable() { return false; }
			};
		}
	}
}