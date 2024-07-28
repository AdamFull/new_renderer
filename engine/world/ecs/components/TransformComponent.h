#pragma once

#include "Component.h"

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			struct FTransformComponent : public IComponent
			{
				// Local transform
				glm::vec3 lposition{ 0.f };
				glm::quat lrotation{ 1.f, 0.f, 0.f, 0.f };
				glm::vec3 lscale{ 1.f };

				// Global transform
				glm::vec3 gposition{ 0.f };
				glm::quat grotation{ 1.f, 0.f, 0.f, 0.f };
				glm::vec3 gscale{ 1.f };

				glm::mat4 gmodel{ 1.f };
				glm::mat4 lmodel{ 1.f };
				glm::mat4 oldmodel{ 1.f };
				glm::mat4 normal{ 1.f };
			};
		}
	}
}

/*
* {
*	images: [],
*	textures: [],
*	materials: [],
*	
*	root: {
*		components: {
*			base_component: {
*				name: "bla_bla",
*				visible: true,
*				enabled: true
*			},
*			transform_component: {
*				pos: [],
*				rot: [],
*				scale: [],
*				mat: []
*			},
*			hierarchy_component: {
*			}
*		}
*	}
* }
*/