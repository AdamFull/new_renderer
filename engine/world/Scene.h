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
		private:
			CSceneGraph* m_pSceneGraph{ nullptr };
			easygltf::CGltfProject* m_pScene{ nullptr };

			std::vector<size_t> m_loadedTextures{};

			render::IRenderInterface* m_pRenderInterface{ nullptr };
			filesystem::IVirtualFileSystemInterface* m_pVFS{ nullptr };
		};
	}
}