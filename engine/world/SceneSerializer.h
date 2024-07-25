#pragma once

#include <functional>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <standart_library.h>

#include "ecs/components/components.h"

namespace glm
{
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ivec2, x, y);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(vec2, x, y);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(dvec2, x, y);

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ivec3, x, y, z);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(vec3, x, y, z);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(dvec3, x, y, z);

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ivec4, x, y, z, w);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(vec4, x, y, z, w);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(dvec4, x, y, z, w);

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(quat, w, x, y, z);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(dquat, w, x, y, z);
}

namespace engine
{
	namespace world
	{
		namespace ecs
		{
			NLOHMANN_JSON_SERIALIZE_ENUM(ECameraType,
				{
					{ECameraType::eOrthographic, "ortho"},
					{ECameraType::ePerspective, "perspective"}
				})

			NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FRootComponent, entity);
			NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FBaseComponent, name, enabled, visible);
			NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FCameraComponent, type, fieldOfView, nearPlane, farPlane, aspectRatio, xmag, ymag);
			NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FDirectionalLightComponent, color, intencity, castShadows);
			NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FHierarchyComponent, parent, children);
			NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FMeshComponent, castShadows);
			NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FPointLightComponent, color, intencity, radius, castShadows);
			NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FSpotLightComponent, color, target, intencity, innerAngle, outerAngle, toTarget, castShadows);
			NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FTransformComponent, lposition, lrotation, lscale);
		}

		class CSceneSerializer
		{
		public:
			using serialize_function_t = std::function<void(entt::registry&, entt::entity, nlohmann::json&)>;
			using deserialize_function_t = std::function<void(entt::registry&, entt::entity, const nlohmann::json&)>;

			template<typename _Ty>
			static void RegisterComponent()
			{
				entt::id_type type = entt::type_hash<_Ty>::value();
				auto string_type = std::to_string(type);
				m_componentSerializers[type] = [string_type](entt::registry& registry, entt::entity entity, nlohmann::json& j) {
					if (registry.all_of<_Ty>(entity))
						j[string_type] = registry.get<_Ty>(entity);
					};
				m_componentDeserializers[type] = [string_type](entt::registry& registry, entt::entity entity, const nlohmann::json& j) {
					if (j.contains(string_type))
						registry.emplace_or_replace<_Ty>(entity, j.at(string_type).get<_Ty>());
					};
			}

			static entt::entity CreateNode(entt::registry& registry, entt::entity parent = entt::null, const std::string& name = "");
			static void DestroyNode(entt::registry& registry, entt::entity node);
			static void AttachChild(entt::registry& registry, entt::entity parent, entt::entity child);
			static void DetachChild(entt::registry& registry, entt::entity parent, entt::entity child);
			static void DetachChild(entt::registry& registry, entt::entity child);
			static entt::entity Instantiate(entt::registry& registry, entt::entity prefab);

			static nlohmann::json SerializeEntity(entt::registry& registry, entt::entity entity);
			static entt::entity DeserializeEntity(entt::registry& registry, entt::entity parent, const nlohmann::json& j);

		private:
			static utl::hash_map<entt::id_type, serialize_function_t> m_componentSerializers;
			static utl::hash_map<entt::id_type, deserialize_function_t> m_componentDeserializers;
		};
	}
}