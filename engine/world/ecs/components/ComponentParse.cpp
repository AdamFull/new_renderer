#include "components.h"

#include <engine/integrations/imgui/EditorElements.h>

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

			//NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FRootComponent, entity);
			//NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FBaseComponent, name, enabled, visible);
			//NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FCameraComponent, type, fieldOfView, nearPlane, farPlane, aspectRatio, xmag, ymag);
			//NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FDirectionalLightComponent, color, intencity, castShadows);
			//NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FHierarchyComponent, parent, children);
			//NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FMeshComponent, castShadows);
			//NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FPointLightComponent, color, intencity, radius, castShadows);
			//NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FSpotLightComponent, color, target, intencity, innerAngle, outerAngle, toTarget, castShadows);
			//NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(FTransformComponent, lposition, lrotation, lscale);
		}
	}
}

using namespace engine;
using namespace engine::world::ecs;

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