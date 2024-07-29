#include "components.h"

#include <engine/integrations/imgui/EditorElements.h>

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
	}
}

using namespace engine;
using namespace engine::world::ecs;

template<typename _Ty>
void SerializeField(const char* field, nlohmann::json& j, const _Ty& value, const _Ty default_value = _Ty{})
{
	if (value != default_value)
		j[field] = value;
}

template<typename _Ty>
void DeserializeField(const char* field, const nlohmann::json& j, _Ty& value, bool expected = false)
{
	if (auto obj = j.find(field); obj != j.end())
		obj->get_to(value);
	else if(expected)
		throw std::runtime_error("Expected field was not found!");
}

#define SERIALIZE_FIELD(field) SerializeField(#field, j, field)
#define DESERIALIZE_FIELD(field) DeserializeField(#field, j, field)
#define DESERIALIZE_EXPECTED_FIELD(field) DeserializeField(#field, j, field, true)

void FCameraComponent::Serialize(nlohmann::json& j)
{
	SERIALIZE_FIELD(type);
	SERIALIZE_FIELD(fieldOfView);
	SERIALIZE_FIELD(nearPlane);
	SERIALIZE_FIELD(farPlane);
	SERIALIZE_FIELD(aspectRatio);
	SERIALIZE_FIELD(xmag);
	SERIALIZE_FIELD(ymag);
}

void FCameraComponent::Deserialize(const nlohmann::json& j)
{
	DESERIALIZE_FIELD(type);
	DESERIALIZE_FIELD(fieldOfView);
	DESERIALIZE_FIELD(nearPlane);
	DESERIALIZE_FIELD(farPlane);
	DESERIALIZE_FIELD(aspectRatio);
	DESERIALIZE_FIELD(xmag);
	DESERIALIZE_FIELD(ymag);
}

void FCameraComponent::OnEdit()
{
	ComponentEdit<ecs::FCameraComponent>("Camera", [this]() {
		ImGui::GDragFloat<1>("FoV", &fieldOfView, 0.01f, 0.01f, 180.f);
		ImGui::GDragFloat<1>("Near", &nearPlane, 0.01f, 0.01f);
		ImGui::GDragFloat<1>("Far", &farPlane, 0.01f, 0.01f);
		ImGui::GDragFloat<1>("Aspect", &aspectRatio, 0.01f);
		//ImGui::GDragFloat("Sensitivity", &sensitivity, 0.01f, 1.f, 50.f);
		ImGui::GCheckbox("Active", &active);
		});
}


void FDirectionalLightComponent::OnEdit()
{
	ComponentEdit<ecs::FDirectionalLightComponent>("Directional light", [this]() {
		ImGui::GColorEdit3("Color", color);
		ImGui::GDragFloat<1>("Intencity", &intencity, 0.01f, 0.01f, 50.f);
		ImGui::GCheckbox("Cast shadows", &castShadows);
		});
}

void FMeshComponent::OnEdit()
{
	ComponentEdit<ecs::FMeshComponent>("Mesh renderer", [this]() {
		ImGui::GCheckbox("Cast shadows", &castShadows);
		});
}

void FPointLightComponent::OnEdit()
{
	ComponentEdit<ecs::FPointLightComponent>("Point light", [this]() {
		ImGui::GColorEdit3("Color", color);
		ImGui::GDragFloat<1>("Intencity", &intencity, 0.01f, 0.01f, 50.f);
		ImGui::GDragFloat<1>("Radius", &radius, 0.01f, 0.01f);
		ImGui::GCheckbox("Cast shadows", &castShadows);
		});
}

void FSpotLightComponent::OnEdit()
{
	ComponentEdit<ecs::FSpotLightComponent>("Spot light", [this]() {
		ImGui::GColorEdit3("Color", color);
		ImGui::GDragFloat<3>("Target", &target[0], 0.01f);
		ImGui::GDragFloat<1>("Intencity", &intencity, 0.01f, 0.01f, 50.f);
		ImGui::GDragFloat<1>("Inner angle", &innerAngle, 0.01f, 0.01f, outerAngle);
		ImGui::GDragFloat<1>("Outer angle", &outerAngle, 0.01f, innerAngle, 3.14f);
		ImGui::GCheckbox("To target", &toTarget);
		ImGui::GCheckbox("Cast shadows", &castShadows);
		});
}

void FTransformComponent::OnEdit()
{
	DefaultComponentEdit<ecs::FTransformComponent>("Transform", [this]() {
		ImGui::GDragFloat<3>("Position", &lposition[0], 0.1f);
		ImGui::GDragFloat<4>("Rotation", &lrotation[0], 0.01f, -1.f, 1.f);
		ImGui::GDragFloat<3>("Scale", &lscale[0], 0.1f);
		});
}