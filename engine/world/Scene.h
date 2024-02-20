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
			stl::unordered_map<u64, stl::vector<u64>> vertexHandlers{};

			stl::vector<u64> textures{};
			u64 lastTexture{ 0ull };

			stl::vector<u64> materials{};
			u64 lastMaterial{ 0ull };
		};

		struct FSceneTemporaryResources
		{
			stl::vector<stl::string> materialNames{};
			stl::unordered_map<u32, entt::entity> nodeToEntity{};
		};

		class CScene
		{
		public:
			CScene(filesystem::IVirtualFileSystemInterface* vfs, render::IRenderInterface* render_interface);
			~CScene();

			void load(const stl::string& path);
			void makeGenerateScene(const stl::string& path);

			void update(f32 dt);
		protected:
			void loadGLTF(easygltf::FGLTFRoot& root);
			void loadNode(const easygltf::FGLTFRoot root, const entt::entity parent, u32 node_index, f32 global_scale = 1.f);

			void loadMeshComponent(const easygltf::FGLTFRoot root, const entt::entity parent, u32 node_index);
			void loadCameraComponent(const easygltf::FGLTFRoot root, const entt::entity parent, u32 node_index);
			void loadLightComponent(const easygltf::FGLTFRoot root, const entt::entity parent, u32 node_index);

			void loadAnimations(const easygltf::FGLTFRoot root);
			void loadSkins(const easygltf::FGLTFRoot root);
		private:
			CSceneGraph* m_pSceneGraph{ nullptr };

			stl::vector<stl::unique_ptr<ecs::ISystem>> m_systems{};

			FSceneLoadedResources m_resources{};
			FSceneTemporaryResources m_temporary{};

			render::IRenderInterface* m_pRenderInterface{ nullptr };
			filesystem::IVirtualFileSystemInterface* m_pVFS{ nullptr };
		};
	}
}