#include <edge/filesystem/VirtualFileSystemInterface.h>
#include <engine/rendering/RendererInterface.h>
#include "ecs/systems/BaseSystem.h"

#include <libraries/easygltf/include/easygltf/easygltf_base.h>

#include <entt/fwd.hpp>

namespace engine
{
	namespace world
	{
		struct SceneLoadedResources
		{
			std::unordered_map<uint32_t, std::vector<uint32_t>> vertexHandlers{};

			std::vector<uint32_t> textures{};
			std::vector<uint32_t> materials{};
		};

		class Scene final
		{
		public:
			Scene(edge::filesystem::IVirtualFileSystemInterface* vfs, render::IRenderer& renderer);
			~Scene();

			void Load(std::string_view path);

			void Update(float delta_time);
		private:
			void LoadGLTF(easygltf::FGLTFRoot& root);

			void LoadNode(const easygltf::FGLTFRoot& root, const entt::entity parent, uint32_t node_index, f32 global_scale = 1.f);

			void LoadMeshComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, uint32_t node_index);
			void LoadCameraComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, uint32_t node_index);
			void LoadLightComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, uint32_t node_index);

			void LoadAnimations(const easygltf::FGLTFRoot& root);
			void LoadSkins(const easygltf::FGLTFRoot& root);
		private:
			render::IRenderer& renderer;

			edge::filesystem::IVirtualFileSystemInterface* vfs{ nullptr };

			SceneLoadedResources resources{};
			std::unordered_map<uint32_t, entt::entity> node_to_entity{};
		};
	}
}