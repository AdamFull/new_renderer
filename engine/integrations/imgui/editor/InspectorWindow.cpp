#include "InspectorWindow.h"

#include <engine/world/EntityManager.h>
#include <engine/integrations/imgui/EditorState.h>
#include <engine/integrations/imgui/EditorTraits.h>
#include <engine/integrations/imgui/ImGuiElements.h>

#include <engine/filesystem/vfs_helper.h>
#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

using namespace engine;
using namespace engine::filesystem;
using namespace engine::world;

template<class _Ty>
void TryAddMenuItemI(const std::string& name, const entt::entity& entity)
{
	//auto& actionBuffer = EGEditor->getActionBuffer();
	if (!CEntityManager::TryGetComponent<_Ty>(entity))
	{
		if (ImGui::MenuItem(_Ty::_Name.data()));
			//actionBuffer->addOperation(std::make_unique<CAddComponentOperation>(entity, utl::type_hash<_Ty>()));
	}
}


void CInspectorWindow::Create()
{

}

void CInspectorWindow::__draw(float fDt)
{
	auto& state = CEditorState::GetInstance();
	//auto& imageLoader = graphics->getImageLoader();
	//auto& debug_draw = graphics->getDebugDraw();
	auto selected = state->GetSelection();

	// TODO: add replace component operation
	if (selected != entt::null)
	{
		auto& transform = CEntityManager::GetComponent<ecs::FTransformComponent>(selected);
		auto& base = CEntityManager::GetComponent<ecs::FBaseComponent>(selected);
		auto& hierarchy = CEntityManager::GetComponent<ecs::FHierarchyComponent>(selected);

		if (ImGui::BeginPopup("add_component_popup"))
		{
			//TryAddMenuItemI<ecs::FAudioComponent>("Audio", selected);
			TryAddMenuItemI<ecs::FCameraComponent>("Camera", selected);
			//TryAddMenuItemI<ecs::FScriptComponent>("Script", selected);
			//TryAddMenuItemI<ecs::FEnvironmentComponent>("Environment", selected);
			TryAddMenuItemI<ecs::FDirectionalLightComponent>("Directional light", selected);
			TryAddMenuItemI<ecs::FSpotLightComponent>("Spot light", selected);
			TryAddMenuItemI<ecs::FPointLightComponent>("Point light", selected);
			//TryAddMenuItemI<ecs::FSceneComponent>("Scene", selected);
			//TryAddMenuItemI<ecs::FRigidBodyComponent>("RigidBody", selected);
			//TryAddMenuItemI<ecs::FParticleComponent>("Particle system", selected);
			//TryAddMenuItemI<ecs::FTerrainComponent>("Terrain", selected);
			ImGui::EndPopup();
		}

		ImGui::Separator();

		ImGui::GTextInput("Name", &base.name);
		ImGui::Separator();

		CEntityManager::IterateComponents(selected, [](ecs::IComponent* component) {component->OnEdit(); });

		if (ImGui::Button(ICON_MDI_PLUS, ImVec2(-1.f, 0.f)))
			ImGui::OpenPopup("add_component_popup");

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const char* path = (const char*)payload->Data;
				auto source = std::string(path);

				if (fs::is_audio_format(source))
				{
					//if (!CEntityManager::TryGetComponent<ecs::FAudioComponent>(selected))
					//{
					//	ecs::FAudioComponent naudio{};
					//	naudio.source = source;
					//
					//	CEntityManager::AddComponent<ecs::FAudioComponent>(selected, std::move(naudio));
					//}
				}
				else if (fs::is_script_format(source))
				{
					//if (!CEntityManager::TryGetComponent<ecs::FScriptComponent>(selected))
					//{
					//	ecs::FScriptComponent nscript{};
					//	nscript.source = source;
					//
					//	CEntityManager::AddComponent<ecs::FScriptComponent>(selected, std::move(nscript));
					//}
				}
			}
			ImGui::EndDragDropTarget();
		}
	}
}

//void CInspectorWindow::AudioEdit(ecs::FAudioComponent* object)
//{
//
//}

void CInspectorWindow::CameraEdit(world::ecs::FCameraComponent* object)
{

}

//void CInspectorWindow::ScriptEdit(world::ecs::FScriptComponent* object)
//{
//
//}
//void CInspectorWindow::SkyboxEdit(world::ecs::FEnvironmentComponent* object) 
//{
//
//}
//void CInspectorWindow::SceneEdit(world::ecs::FSceneComponent* object) 
//{
//
//}
//void CInspectorWindow::RigidbodyEdit(world::ecs::FTransformComponent* transform, world::ecs::FRigidBodyComponent* object) 
//{
//
//}
//void CInspectorWindow::ParticleSystemEdit(world::ecs::FTransformComponent* transform, world::ecs::FParticleComponent* object) 
//{
//
//}
//void CInspectorWindow::TerrainEdit(world::ecs::FTerrainComponent* object) 
//{
//
//}
		
void CInspectorWindow::MeshEdit(world::ecs::FMeshComponent* object)
{

}