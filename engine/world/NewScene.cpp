#include <engine/world/NewScene.h>

#include <engine/world/ecs/components/components.h>
#include <engine/world/ecs/systems/systems.h>

#include<easygltf/easygltf_loader.h>
#include<easygltf/easygltf_helpers.h>

#include <meshoptimizer.h>

#include <engine/rendering/Rendering.h>
#include <engine/rendering/gfx/Structures.h>
#include <engine/rendering/gfx/Enums.h>

#include <engine/filesystem/vfs_helper.h>
#include <util/Logger.h>
#include <glm/gtc/type_ptr.hpp>

#include "EntityManager.h"

namespace engine
{
	namespace world
	{
		class VFSWrapper : public easygltf::IFilesystemWrapper
		{
		public:
			VFSWrapper(filesystem::IVirtualFileSystemInterface* vfs) : vfs(vfs) {}
			~VFSWrapper() override = default;

			bool exists(const std::string& path) override
			{
				return vfs->Exists(path);
			}

			std::string expandFilePath(const std::string& path) override
			{
				return filesystem::fs::normalize(filesystem::fs::to_posix_path(path));
			}

			bool readFile(const std::string& path, utl::vector<uint8_t>& data) override
			{
				return vfs->ReadFile(path, data);
			}

			bool writeFile(const std::string& path, const utl::vector<uint8_t>& data) override
			{
				return vfs->WriteFile(path, data);
			}

			u64 fileSize(const std::string& path) override
			{
				return vfs->FileSize(path);
			}
		private:
			filesystem::IVirtualFileSystemInterface* vfs{ nullptr };
		};

		gfx::Filter GetSamplerFilter(int32_t filter)
		{
			gfx::Filter gfx_filter{ gfx::Filter::eNearest };

			switch (filter)
			{
			case easygltf::base::enumerators::texture_filter::nearest: 
			case easygltf::base::enumerators::texture_filter::nearest_mipmap_nearest:
			case easygltf::base::enumerators::texture_filter::nearest_mipmap_linear:
				gfx_filter = gfx::Filter::eNearest;
				break;
			case easygltf::base::enumerators::texture_filter::linear:
			case easygltf::base::enumerators::texture_filter::linear_mipmap_nearest:
			case easygltf::base::enumerators::texture_filter::linear_mipmap_linear:
				gfx_filter = gfx::Filter::eLinear;
				break;
			default:
				break;
			}

			return gfx_filter;
		}

		gfx::SamplerAddressMode GetSamplerAddressMode(int32_t address_mode)
		{
			gfx::SamplerAddressMode gfx_address_mode{};

			switch (address_mode)
			{
			case easygltf::base::enumerators::texture_wrap::repeat:
				gfx_address_mode = gfx::SamplerAddressMode::eRepeat;
				break;
			case easygltf::base::enumerators::texture_wrap::clamp_to_edge:
				gfx_address_mode = gfx::SamplerAddressMode::eClampToEdge;
				break;
			case easygltf::base::enumerators::texture_wrap::mirrored_repeat:
				gfx_address_mode = gfx::SamplerAddressMode::eMirror;
				break;
			default:
				break;
			}

			return gfx_address_mode;
		}

		gfx::SamplerCreateInfo GetSamplerCreateInfo(const easygltf::Sampler& sampler)
		{
			gfx::SamplerCreateInfo create_info{};

			create_info.min_filter = GetSamplerFilter(sampler.minFilter);
			create_info.mag_filter = GetSamplerFilter(sampler.magFilter);
			create_info.mip_map_mode = GetSamplerFilter(sampler.minFilter);

			create_info.address_u = GetSamplerAddressMode(sampler.wrapS);
			create_info.address_v = GetSamplerAddressMode(sampler.wrapT);

			return create_info;
		}

		Scene::Scene(filesystem::IVirtualFileSystemInterface* vfs, render::Renderer& renderer) :
			renderer{ renderer },
			vfs{ vfs }
		{
			CEntityManager::RegisterSystem<ecs::CInputSystem>();
			CEntityManager::RegisterSystem<ecs::CAnimationSystem>();
			CEntityManager::RegisterSystem<ecs::CScenegraphSystem>();
			//CEntityManager::RegisterSystem<ecs::CSkinningSystem>(render_interface);

			//CEntityManager::RegisterSystem<ecs::CCameraSystem>(render_interface);
			//CEntityManager::RegisterSystem<ecs::CShadowsSystem>(render_interface);
			//CEntityManager::RegisterSystem<ecs::CLightingSystem>(render_interface);
			//CEntityManager::RegisterSystem<ecs::CMeshSystem>(render_interface);
		}

		Scene::~Scene()
		{

		}

		void Scene::Load(const std::string& path)
		{
			easygltf::CGLTFLoader loader{};
			loader.setFilesystemWrapper(std::make_unique<VFSWrapper>(vfs));

			easygltf::FGLTFRoot root{};
			if (!loader.load(path, root))
				return;

			LoadGLTF(root);
		}

		void Scene::LoadGLTF(easygltf::FGLTFRoot& root)
		{
			// Load textures
			for (const auto& texture : root.textures)
			{
				i32 texture_source{ texture.source };
				if (texture.source < 0)
					texture_source = texture.khrTextureBasisU.source;

				auto& image = root.images[texture_source];
				auto& sampler = root.samplers[texture.sampler];

				auto sampler_create_info = GetSamplerCreateInfo(sampler);
				gfx::Sampler* texture_sampler = renderer.GetOrCreateSampler(sampler_create_info);

				// TODO: add ability to override format and sampler params
				// TODO: find format from materials
				auto image_path = filesystem::fs::path_append(root.rootDirectory, easygltf::url_decode(image.uri));
				resources.textures.emplace_back(renderer.LoadTexture(image_path, texture_sampler).Get());
			}

			utl::vector<render::FMaterial> materials{};
			for (u64 idx = 0ull; idx < root.materials.size(); ++idx)
			{
				auto& material = root.materials[idx];
				auto& material_data = materials.emplace_back();

				temporary.materialNames.emplace_back(material.name);
				// TODO: add my own ext that will contain shader EXT_material_shader or something. But in my case it will contain name in format {library}:{shader_name} and other helpfull data

				// Attach base color (aka albedo)
				if (auto& texture = material.pbrMetallicRoughness.baseColorTexture; texture.index >= 0)
					material_data.baseColorTexture = resources.textures[texture.index];

				// Attach metallic roughness texture (metallic = green, roughness = blue)
				if (auto& texture = material.pbrMetallicRoughness.metallicRoughnessTexture; texture.index >= 0)
					material_data.metallicRoughnessTexture = resources.textures[texture.index];

				// Attach normal map texture
				if (auto& texture = material.normalTexture; texture.index >= 0)
				{
					material_data.normalScale = utl::cast<f32>(texture.scale);
					material_data.normalTexture = resources.textures[texture.index];
				}

				// Attach baked ambient occlusion texture
				if (auto& texture = material.occlusionTexture; texture.index >= 0)
				{
					material_data.occlusionStrenth = utl::cast<f32>(texture.strength);
					material_data.occlusionTexture = resources.textures[texture.index];
				}

				// Attach emissive texture
				if (auto& texture = material.emissiveTexture; texture.index >= 0)
					material_data.emissiveTexture = resources.textures[texture.index];

				material_data.emissiveFactor = glm::make_vec3(material.emissiveFactor.data());
				material_data.roughnessFactor = utl::cast<f32>(material.pbrMetallicRoughness.roughnessFactor);
				material_data.metallicFactor = utl::cast<f32>(material.pbrMetallicRoughness.metallicFactor);
				material_data.baseColorFactor = glm::make_vec4(material.pbrMetallicRoughness.baseColorFactor.data());

				if (material.alphaMode == easygltf::base::enumerators::alpha_mode::blend)
					material_data.alphaMode = utl::cast<u32>(render::EAlphaMode::eBlend);
				else if (material.alphaMode == easygltf::base::enumerators::alpha_mode::mask)
					material_data.alphaMode = utl::cast<u32>(render::EAlphaMode::eMask);
				else
					material_data.alphaMode = utl::cast<u32>(render::EAlphaMode::eOpaque);

				material_data.alphaCutoff = utl::cast<f32>(material.alphaCutoff);
				material_data.doubleSided = material.doubleSided;
			}
		}

		void Scene::LoadNode(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index, f32 global_scale)
		{

		}

		void Scene::LoadMeshComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index)
		{

		}

		void Scene::LoadCameraComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index)
		{

		}

		void Scene::LoadLightComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index)
		{

		}

		void Scene::LoadAnimations(const easygltf::FGLTFRoot& root)
		{

		}

		void Scene::LoadSkins(const easygltf::FGLTFRoot& root)
		{

		}
	}
}