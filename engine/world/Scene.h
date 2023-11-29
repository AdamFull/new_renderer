#pragma once

#include <interface/filesystem/VirtualFileSystemInterface.h>
#include <interface/render/RenderInterface.h>
#include <world/ecs/systems/BaseSystem.h>

#include <entt/fwd.hpp>

namespace easygltf
{
	class CGltfProject;
}

namespace engine
{
	namespace world
	{
		class CSceneGraph;

		struct FSceneLoadedResources
		{
			std::unordered_map<size_t, std::vector<size_t>> vertexHandlers{};

			std::vector<size_t> textures{};
			size_t lastTexture{ 0ull };

			std::vector<size_t> materials{};
			size_t lastMaterial{ 0ull };

			size_t primitiveCount{ 0ull };
		};

		struct FSceneTemporaryResources
		{
			std::vector<std::string> materialNames{};
			std::unordered_map<uint32_t, entt::entity> nodeToEntity{};
		};

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

			FSceneLoadedResources m_resources{};
			FSceneTemporaryResources m_temporary{};

			render::IRenderInterface* m_pRenderInterface{ nullptr };
			filesystem::IVirtualFileSystemInterface* m_pVFS{ nullptr };
		};
	}
}