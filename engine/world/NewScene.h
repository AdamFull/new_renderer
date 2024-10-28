#include <engine/interface/filesystem/VirtualFileSystemInterface.h>
#include <engine/rendering/RendererInterface.h>
#include <engine/world/ecs/systems/BaseSystem.h>

#include <libraries/easygltf/include/easygltf/easygltf_base.h>

#include <entt/fwd.hpp>

namespace engine
{
	namespace world
	{
		struct SceneLoadedResources
		{
			utl::hash_map<uint32_t, utl::vector<uint32_t>> vertexHandlers{};

			utl::vector<uint32_t> textures{};
			utl::vector<uint32_t> materials{};
		};

		class Scene final
		{
		public:
			Scene(filesystem::IVirtualFileSystemInterface* vfs, render::IRenderer& renderer);
			~Scene();

			void Load(const std::string& path);
		private:
			void LoadGLTF(easygltf::FGLTFRoot& root);

			void LoadNode(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index, f32 global_scale = 1.f);

			void LoadMeshComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index);
			void LoadCameraComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index);
			void LoadLightComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index);

			void LoadAnimations(const easygltf::FGLTFRoot& root);
			void LoadSkins(const easygltf::FGLTFRoot& root);
		private:
			render::IRenderer& renderer;

			filesystem::IVirtualFileSystemInterface* vfs{ nullptr };

			SceneLoadedResources resources{};
			utl::hash_map<u32, entt::entity> node_to_entity{};
		};
	}
}