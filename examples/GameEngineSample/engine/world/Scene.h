#include "../rendering/RendererInterface.h"
#include "ecs/systems/BaseSystem.h"

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include <tiny_gltf.h>

#include <entt/fwd.hpp>

namespace engine
{
	namespace world
	{
		struct SceneLoadedResources
		{
			std::unordered_map<uint32_t, std::vector<uint32_t>> vertexHandlers{};

			std::vector<uint16_t> images{};
			std::vector<uint16_t> samplers{};
			std::vector<uint32_t> materials{};
		};

		class Scene final
		{
		public:
			Scene(render::IRenderer& renderer);
			~Scene();

			void Load(const std::string& path);

			void Update(float delta_time);
		private:
			void LoadGLTF(tinygltf::Model& root);

			void LoadNode(const tinygltf::Model& root, const entt::entity parent, uint32_t node_index, float global_scale = 1.f);

			void LoadMeshComponent(const tinygltf::Model& root, const entt::entity parent, uint32_t node_index);
			void LoadCameraComponent(const tinygltf::Model& root, const entt::entity parent, uint32_t node_index);
			void LoadLightComponent(const tinygltf::Model& root, const entt::entity parent, uint32_t node_index);

			void LoadAnimations(const tinygltf::Model& root);
			void LoadSkins(const tinygltf::Model& root);
		private:
			render::IRenderer& renderer;
			std::string scene_root_directory;

			SceneLoadedResources resources{};
			std::unordered_map<uint32_t, entt::entity> node_to_entity{};
		};
	}
}