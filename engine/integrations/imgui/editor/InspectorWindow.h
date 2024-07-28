#pragma once

#include "EditorWindow.h"
#include <engine/world/ecs/components/components.h>

namespace engine
{
	class CInspectorWindow : public IEditorWindow
	{
	public:
		CInspectorWindow(const std::string& sname) { m_name = sname; }
		virtual ~CInspectorWindow() override = default;

		void Create() override;
	private:
		void __draw(float fDt) override;

		//void AudioEdit(ecs::FAudioComponent* object);
		void CameraEdit(world::ecs::FCameraComponent* object);
		//void ScriptEdit(world::ecs::FScriptComponent* object);
		//void SkyboxEdit(world::ecs::FEnvironmentComponent* object);
		//void SceneEdit(world::ecs::FSceneComponent* object);
		//void RigidbodyEdit(world::ecs::FTransformComponent* transform, world::ecs::FRigidBodyComponent* object);
		//void ParticleSystemEdit(world::ecs::FTransformComponent* transform, world::ecs::FParticleComponent* object);
		//void TerrainEdit(world::ecs::FTerrainComponent* object);
		void MeshEdit(world::ecs::FMeshComponent* object);
	};
}