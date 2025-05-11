#include "../rendering/RendererInterface.h"
#include "ecs/systems/BaseSystem.h"

#include <entt/fwd.hpp>

struct cgltf_options;
struct cgltf_data;
struct cgltf_node;
struct cgltf_mesh;
struct cgltf_camera;
struct cgltf_light;

namespace engine
{
	namespace world
	{
		struct SceneLoadedResources {
			std::unordered_map<uint32_t, std::vector<uint32_t>> vertexHandlers{};

			std::vector<uint16_t> images{};
			std::vector<uint16_t> samplers{};
			std::vector<uint32_t> materials{};

			std::vector<interop::Vertex> vertices{};
			std::vector<uint32_t> indices{};
		};

		class Scene final
		{
		public:
			Scene(render::IRenderer& renderer);
			~Scene();

			void Load(const std::string& path);

			void Update(float delta_time);
		private:
			void load_node(const cgltf_node* node, const entt::entity parent);
			void load_mesh(const cgltf_mesh* mesh, const entt::entity parent);
			void load_camera(const cgltf_camera* mesh, const entt::entity parent);
			void load_light(const cgltf_light* mesh, const entt::entity parent);

			//void LoadAnimations(const tinygltf::Model& root);
			//void LoadSkins(const tinygltf::Model& root);
		private:
			render::IRenderer& renderer;
			std::string scene_root_directory;

			cgltf_options* gltf_options{ nullptr };
			cgltf_data* gltf_data{ nullptr };

			SceneLoadedResources resources{};
			std::unordered_map<uint32_t, entt::entity> node_to_entity{};
		};
	}
}