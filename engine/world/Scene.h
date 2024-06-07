#pragma once

#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/interface/render/RenderInterface.h>
#include <engine/world/ecs/systems/BaseSystem.h>

#include <libraries/easygltf/include/easygltf/easygltf_base.h>

#include <entt/fwd.hpp>

namespace engine
{
	namespace world
	{
		class CSceneGraph;

		struct FSceneLoadedResources
		{
			utl::hash_map<u64, utl::vector<render::FRHIHandle>> vertexHandlers{};

			utl::vector<u64> textures{};
			u64 lastTexture{ 0ull };

			utl::vector<u64> materials{};
			u64 lastMaterial{ 0ull };
		};

		struct FSceneTemporaryResources
		{
			utl::vector<std::string> materialNames{};
			utl::hash_map<u32, entt::entity> nodeToEntity{};
		};

		class CScene
		{
		public:
			CScene(filesystem::IVirtualFileSystemInterface* vfs, render::IRenderInterface* render_interface);
			~CScene();

			void Load(const std::string& path);
			void MakeGenerateScene(const std::string& path);

			void Update(f32 dt);
		protected:
			void LoadGLTF(easygltf::FGLTFRoot& root);
			void LoadNode(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index, f32 global_scale = 1.f);

			void LoadMeshComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index);
			void LoadCameraComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index);
			void LoadLightComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index);

			void LoadAnimations(const easygltf::FGLTFRoot& root);
			void LoadSkins(const easygltf::FGLTFRoot& root);
		private:
			CSceneGraph* m_pSceneGraph{ nullptr };

			utl::vector<std::unique_ptr<ecs::ISystem>> m_systems{};

			FSceneLoadedResources m_resources{};
			FSceneTemporaryResources m_temporary{};

			render::IRenderInterface* m_pRenderInterface{ nullptr };
			filesystem::IVirtualFileSystemInterface* m_pVFS{ nullptr };
		};
	}
}