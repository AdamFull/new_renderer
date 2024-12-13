#pragma once

#include <engine/ui/font/IconsMaterialDesignIcons.h>
#include <engine/world/ecs/components/components.h>

namespace engine
{
	template<typename _Ty>
	struct IconByComponentType;

	template<>
	struct IconByComponentType<entt::entity>
	{
		static constexpr const char* value = ICON_MDI_GRAPH;
	};

	template<>
	struct IconByComponentType<world::ecs::FTransformComponent>
	{
		static constexpr const char* value = ICON_MDI_AXIS; /*ICON_MDI_VECTOR_LINE*/
	};

	template<>
	struct IconByComponentType<world::ecs::FSpotLightComponent>
	{
		static constexpr const char* value = ICON_MDI_HEAD_LIGHTBULB;
	};

	template<>
	struct IconByComponentType<world::ecs::FPointLightComponent>
	{
		static constexpr const char* value = ICON_MDI_LIGHTBULB;
	};

	template<>
	struct IconByComponentType<world::ecs::FDirectionalLightComponent>
	{
		static constexpr const char* value = ICON_MDI_BRIGHTNESS_7;
	};

	//template<>
	//struct IconByComponentType<ecs::FAudioComponent>
	//{ static constexpr const char* value = ICON_MDI_VOLUME_HIGH; };

	template<>
	struct IconByComponentType<world::ecs::FCameraComponent>
	{
		static constexpr const char* value = ICON_MDI_CAMERA;
	};

	template<>
	struct IconByComponentType<world::ecs::FMeshComponent>
	{
		static constexpr const char* value = ICON_MDI_SHAPE;
	};

	//template<>
	//struct IconByComponentType<world::ecs::FRigidBodyComponent>
	//{ static constexpr const char* value = ICON_MDI_SHAPE; };

	//template<>
	//struct IconByComponentType<world::ecs::FScriptComponent>
	//{ static constexpr const char* value = ICON_MDI_SCRIPT; };
	//
	//template<>
	//struct IconByComponentType<world::ecs::FEnvironmentComponent>
	//{ static constexpr const char* value = ICON_MDI_EARTH; };
	//
	//template<>
	//struct IconByComponentType<world::ecs::FSpriteComponent>
	//{ static constexpr const char* value = ICON_MDI_IMAGE; };
	//
	//template<>
	//struct IconByComponentType<world::ecs::FParticleComponent>
	//{ static constexpr const char* value = ICON_MDI_CREATION; };
	//
	//template<>
	//struct IconByComponentType<world::ecs::FTerrainComponent>
	//{ static constexpr const char* value = ICON_MDI_CREATION; };

	template<typename _Ty>
	constexpr const char* IconByComponentType_v = IconByComponentType<_Ty>::value;
}