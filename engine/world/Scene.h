#pragma once

#include <world/parser/easygltf_fwd.h>
#include <interface/filesystem/VirtualFileSystemInterface.h>
#include <interface/RenderInterface.h>
#include <world/ecs/systems/BaseSystem.h>

namespace engine
{
	namespace world
	{
		class CSceneGraph;

		class CScene
		{
		public:
			CScene(filesystem::IVirtualFileSystemInterface* vfs, render::IRenderInterface* render_interface);
			~CScene();

			void load(const std::string& path);

			void update(float dt);
		protected:
			void loadTextures();
			void loadMaterials();

			void loadNode(const entt::entity parent, uint32_t node_index, float global_scale = 1.f);

			void loadMeshComponent(const entt::entity parent, uint32_t node_index);
			void loadCameraComponent(const entt::entity parent, uint32_t node_index);
			void loadLightComponent(const entt::entity parent, uint32_t node_index);

			void loadAnimations();
			void loadSkins();
		private:
			CSceneGraph* m_pSceneGraph{ nullptr };
			easygltf::CGltfProject* m_pScene{ nullptr };

			std::vector<std::unique_ptr<ecs::ISystem>> m_systems{};

			std::unordered_map<uint32_t, entt::entity> m_nodeToEntityMap{};
			size_t m_primitiveCount{ 0ull };

			std::vector<size_t> m_loadedTextures{};

			std::unique_ptr<render::IVertexHandler> m_pVertexHandler{};

			render::IRenderInterface* m_pRenderInterface{ nullptr };
			filesystem::IVirtualFileSystemInterface* m_pVFS{ nullptr };
		};
	}
}