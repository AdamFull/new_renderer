#pragma once

#include "SceneGraph.h"
#include <interface/filesystem/VirtualFileSystemInterface.h>

namespace engine
{
	namespace world
	{
		class CScene
		{
		public:
			CScene(filesystem::IVirtualFileSystemInterface* vfs);

			void create();
			void load(const std::string& path);
		protected:
		private:
			std::unique_ptr<CSceneGraph> m_pSceneGraph{};
			entt::entity m_root{};

			filesystem::IVirtualFileSystemInterface* m_pVFS{ nullptr };
		};
	}
}