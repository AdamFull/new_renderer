#pragma once

#include <functional>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <standart_library.h>

#include <engine/world/ecs/systems/BaseSystem.h>
#include <engine/world/ecs/components/components.h>

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

		class CEntityManager
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

			template<typename _Ty, typename... _Args>
			static void RegisterSystem(_Args&&... args)
			{
				m_vSystems.emplace_back(std::make_unique<_Ty>(std::forward<_Args>(args)...));
			}

			static entt::entity CreateRoot(const std::string& name);
			static entt::entity CreateNode(const std::string& name = "", entt::entity parent = entt::null);
			static void DestroyNode(entt::entity node);
			static void AttachChild(entt::entity parent, entt::entity child);
			static void DetachChild(entt::entity parent, entt::entity child);
			static void DetachChild(entt::entity child);
			static entt::entity Instantiate(entt::entity prefab);

			static bool IsValid(entt::entity entity);

			template<typename... _Components>
			static auto View() 
			{
				return m_registry.view<_Components...>();
			}

			template<typename... _Components, typename _Func>
			static void Each(_Func func) 
			{
				auto v = m_registry.view<_Components...>();
				v.each(func);
			}


			template<typename _Ty>
			static _Ty& AddComponent(entt::entity entity, _Ty&& component)
			{
				return m_registry.emplace<_Ty>(entity, std::forward<_Ty>(component));
			}

			template<typename _Ty>
			static _Ty& GetComponent(entt::entity entity)
			{
				return m_registry.get<_Ty>(entity);
			}

			static entt::registry& GetRegistry() { return m_registry; }
			static entt::entity GetRoot() { return m_root; }

			static nlohmann::json SerializeEntity(entt::entity entity);
			static entt::entity DeserializeEntity(entt::entity parent, const nlohmann::json& j);

			static void Create();
			static void Update(f32 dt);

		private:
			static entt::registry m_registry;
			static entt::entity m_root;
			static utl::vector<std::unique_ptr<ecs::ISystem>> m_vSystems;
			static utl::hash_map<entt::id_type, serialize_function_t> m_componentSerializers;
			static utl::hash_map<entt::id_type, deserialize_function_t> m_componentDeserializers;
		};
	}
}