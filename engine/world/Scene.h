#pragma once

#include "parser/easygltf_fwd.h"
#include <interface/filesystem/VirtualFileSystemInterface.h>
#include <interface/RenderInterface.h>

namespace engine
{
	namespace world
	{
		class CSceneGraph;

		class CScene
		{
		public:
			CScene(filesystem::IVirtualFileSystemInterface* vfs);
			~CScene();

			void load(const std::string& path);
		protected:
			void loadTextures();
			void loadMaterials();

			void loadNode(const uint32_t parent, uint32_t node_index, float global_scale = 1.f);

			void loadMeshComponent(const uint32_t parent, uint32_t node_index);
			void loadCameraComponent(const uint32_t parent, uint32_t node_index);
			void loadLightComponent(const uint32_t parent, uint32_t node_index);
		private:
			CSceneGraph* m_pSceneGraph{ nullptr };
			easygltf::CGltfProject* m_pScene{ nullptr };

			uint32_t m_root{ ~0u };
			std::unordered_map<uint32_t, uint32_t> m_nodeToEntityMap{};

			std::vector<size_t> m_loadedTextures{};

			std::unique_ptr<render::IVertexHandler> m_pVertexHandler{};

			render::IRenderInterface* m_pRenderInterface{ nullptr };
			filesystem::IVirtualFileSystemInterface* m_pVFS{ nullptr };
		};
	}
}