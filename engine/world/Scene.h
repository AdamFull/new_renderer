#pragma once

#include "parser/easygltf_fwd.h"
#include <interface/filesystem/VirtualFileSystemInterface.h>
#include <interface/RenderInterface.h>

namespace engine
{
	namespace world
	{
		struct FVertex
		{
			FVertex() = default;
			FVertex(glm::vec3 p, glm::vec3 c) : pos(p), color(c) {}
			FVertex(glm::vec3 p, glm::vec3 n, glm::vec2 t) : pos(p), normal(n), texcoord(t) {}

			glm::vec3 pos{};
			glm::vec3 color{};
			glm::vec3 normal{};
			glm::vec2 texcoord{};
			glm::vec4 tangent{};
			glm::vec4 joint0{};
			glm::vec4 weight0{};
		};

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
		private:
			CSceneGraph* m_pSceneGraph{ nullptr };
			easygltf::CGltfProject* m_pScene{ nullptr };

			uint32_t m_root{ -1 };
			std::unordered_map<uint32_t, uint32_t> m_nodeToEntityMap{};

			std::vector<size_t> m_loadedTextures{};

			uint32_t m_lastIndex{ 0u };
			uint32_t m_lastVertex{ 0u };

			std::vector<uint8_t> m_indexBuffer;
			std::vector<uint8_t> m_vertexBuffer;

			render::IRenderInterface* m_pRenderInterface{ nullptr };
			filesystem::IVirtualFileSystemInterface* m_pVFS{ nullptr };
		};
	}
}