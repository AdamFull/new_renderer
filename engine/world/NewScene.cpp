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

		Scene::Scene(filesystem::IVirtualFileSystemInterface* vfs, render::IRenderer& renderer) :
			vfs{ vfs },
			renderer{ renderer }
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
				resources.textures.push_back(renderer.AddTextureResource(image_path, texture_sampler));
			}

			utl::vector<render::FMaterial> materials{};
			for (u64 idx = 0ull; idx < root.materials.size(); ++idx)
			{
				auto& material = root.materials[idx];
				auto& material_data = materials.emplace_back();

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

			// Loading nodes
			const easygltf::Scene& scene = root.scenes[root.scene > -1 ? root.scene : 0];
			auto scene_root = CEntityManager::CreateRoot(scene.name);

			for (const auto& node_index : scene.nodes)
				LoadNode(root, scene_root, node_index);

			LoadAnimations(root);
			LoadSkins(root);

			CEntityManager::Create();
		}

		void Scene::LoadNode(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index, f32 global_scale)
		{
			const auto& node = root.nodes[node_index];

			auto new_node = CEntityManager::CreateNode(node.name);

			log_info("[Scene] Loadind node: {}:{}.", node.name, node_index);

			node_to_entity[node_index] = new_node;

			// Parse node local transform
			auto& transform = CEntityManager::GetComponent<ecs::FTransformComponent>(new_node);
			transform.lposition = glm::make_vec3(node.translation.data());
			transform.lrotation = glm::make_quat(node.rotation.data());
			transform.lscale = glm::make_vec3(node.scale.data());
			transform.lmodel = glm::make_mat4(node.matrix.data());

			// Parse node children
			if (!node.children.empty())
			{
				for (const auto& child_node_index : node.children)
					LoadNode(root, node_to_entity[node_index], child_node_index);
			}

			if (node.mesh >= 0)
				LoadMeshComponent(root, node_to_entity[node_index], node_index);

			if (node.camera >= 0)
				LoadCameraComponent(root, node_to_entity[node_index], node_index);

			if (node.hasExtension<easygltf::extensions::KHR_lights_punctual>())
				LoadLightComponent(root, node_to_entity[node_index], node_index);

			CEntityManager::AttachChild(parent, new_node);
		}

		void Scene::LoadMeshComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index)
		{
			const auto& node = root.nodes[node_index];
			const auto& mesh = root.meshes[node.mesh];

			auto& mesh_component = CEntityManager::AddComponent<ecs::FMeshComponent>(parent, ecs::FMeshComponent{});
			mesh_component.skin = node.skin;

			// Load mesh primitives
			for (u64 i = 0ull; i < mesh.primitives.size(); ++i)
			{
				const auto& primitive = mesh.primitives[i];

				// TODO: add min max
				glm::vec3 pos_min{}, pos_max{};

				// Load vertices

				std::hash<u32> attribute_hasher;
				utl::vector<render::FVertexAttributePointer> attributes{};
				u64 position_index{ 0ull };

				u64 attributes_hash{ 0ull };
				for (const auto& [name, accessor_idx] : primitive.attributes)
				{
					if (name == "TEXCOORD_1")
						continue;

					const easygltf::Accessor& accessor = root.accessors[accessor_idx];
					auto stream = root.getStream(accessor);

					auto& attrib = attributes.emplace_back();
					//attrib.type = get_vertex_attribute_type_by_name(name);
					attrib.data = stream.data;
					attrib.stride = stream.stride;
					attrib.count = stream.count;

					if (attrib.type == render::EVertexAttributeType::ePosition)
					{
						pos_min = !accessor.min.empty() ? glm::vec3(accessor.min[0], accessor.min[1], accessor.min[2]) : glm::vec3(0.f);
						pos_max = !accessor.max.empty() ? glm::vec3(accessor.max[0], accessor.max[1], accessor.max[2]) : glm::vec3(0.f);
						position_index = attributes.size() - 1ull;
					}

					attributes_hash ^= attribute_hasher(utl::cast<u32>(attrib.type) + 1u) << 1;
				}

				// Attributes should be sorted by value type
				std::sort(attributes.begin(), attributes.end(), [](const auto& lhs, const auto& rhs) { return utl::cast<u32>(lhs.type) < utl::cast<u32>(rhs.type); });

				// Load indices
				render::FVertexAttributePointer indices{};
				if (primitive.indices >= 0)
				{
					const easygltf::Accessor& accessor = root.accessors[primitive.indices];
					auto stream = root.getStream(accessor);
					indices.data = stream.data;
					indices.stride = stream.stride;
					indices.count = stream.count;

					attributes_hash ^= attribute_hasher(utl::cast<u32>(indices.stride)) << 1;
				}

				// Get or Create vertex buffer
				//render::IVertexHandler* vertex_handler{ nullptr };
				//render::FRHIHandle current_handler_id{};

				//if (auto found = m_resources.vertexHandlers.find(attributes_hash); found != m_resources.vertexHandlers.end())
				//{
				//	for (const auto& handler_id : found->second)
				//	{
				//		auto& handler = m_pRenderInterface->GetVertexHandler(handler_id);
				//
				//		vertex_handler = handler.get();
				//		current_handler_id = handler_id;
				//	}
				//}

				//if (!vertex_handler)
				//{
				//	render::FVertexHandlerCreateInfo handler_ci{};
				//	handler_ci.indexSize = indices.stride;
				//	handler_ci.name = mesh.name;
				//	handler_ci.flags = render::EVertexHandlerFlagBits::eGenerateMeshlets;
				//
				//	for (const auto& attrib : attributes)
				//	{
				//		auto& desc = handler_ci.descriptions.emplace_back();
				//		desc.type = attrib.type;
				//		desc.size = attrib.stride;
				//	}
				//
				//	auto& handlers = m_resources.vertexHandlers[attributes_hash];
				//
				//	auto handler_name = std::format("{}_{}_{}", mesh.name, attributes_hash, handlers.size());
				//	auto handler_id = m_pRenderInterface->CreateVertexHandler(handler_name, handler_ci);
				//	handlers.emplace_back(handler_id);
				//
				//	auto& handler = m_pRenderInterface->GetVertexHandler(handler_id);
				//	vertex_handler = handler.get();
				//	current_handler_id = handler_id;
				//}

				// Create mesh primitive
				//ecs::FMeshPrimitive mesh_primitive{};
				//mesh_primitive.vboId = current_handler_id;
				//mesh_primitive.setDimensions(pos_min, pos_max);
				//mesh_primitive.vertexRange = vertex_handler->AddMeshData(attributes, indices);
				//
				//// NOTE: Updating material Create info for future shader creation
				//if (primitive.material >= 0)
				//	mesh_primitive.materialId = primitive.material;
				//
				//mesh_component.primitives.emplace_back(mesh_primitive);
			}
		}

		void Scene::LoadCameraComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index)
		{
			const auto& node = root.nodes[node_index];
			const auto& camera = root.cameras[node.camera];

		ecs::FCameraComponent camera_component;
			if (camera.type == easygltf::base::enumerators::camera_type::orthographic)
			{
				camera_component.ctype = ecs::ECameraType::eOrthographic;
				camera_component.xmag = camera.orthographic.xmag;
				camera_component.ymag = camera.orthographic.ymag;
				camera_component.nearPlane = camera.orthographic.znear;
				camera_component.farPlane = camera.orthographic.zfar;
			}
			else if (camera.type == easygltf::base::enumerators::camera_type::perspective)
			{
				camera_component.ctype = ecs::ECameraType::ePerspective;
				camera_component.aspectRatio = camera.perspective.aspectRatio;
				camera_component.fieldOfView = glm::degrees(2.0f * glm::atan(glm::tan(camera.perspective.yfov * 0.5f) * camera.perspective.aspectRatio));
				camera_component.nearPlane = camera.perspective.znear;
				camera_component.farPlane = camera.perspective.zfar;
			}

			CEntityManager::AddComponent<ecs::FCameraComponent>(parent, std::move(camera_component));
		}

		void Scene::LoadLightComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index)
		{
			const auto& node = root.nodes[node_index];

			auto& light = root.khrLightsPunctual.lights[node.khrLightsPunctual.light];
			if (light.type == easygltf::extensions::KHR_lights_punctual::enumerators::light_type::directional)
			{
				ecs::FDirectionalLightComponent light_component;
				light_component.color = glm::make_vec3(light.color.data());
				light_component.intencity = light.intensity;
				CEntityManager::AddComponent<ecs::FDirectionalLightComponent>(parent, std::move(light_component));
			}
			else if (light.type == easygltf::extensions::KHR_lights_punctual::enumerators::light_type::spot)
			{
				ecs::FSpotLightComponent light_component;
				light_component.color = glm::make_vec3(light.color.data());
				light_component.innerAngle = light.spot.innerConeAngle;
				light_component.outerAngle = light.spot.outerConeAngle;
				light_component.intencity = light.intensity;
				CEntityManager::AddComponent<ecs::FSpotLightComponent>(parent, std::move(light_component));
			}
			else if (light.type == easygltf::extensions::KHR_lights_punctual::enumerators::light_type::point)
			{
				ecs::FPointLightComponent light_component;
				light_component.color = glm::make_vec3(light.color.data());
				light_component.intencity = light.intensity;
				light_component.radius = (std::max)(light.range, 1.0);
				CEntityManager::AddComponent<ecs::FPointLightComponent>(parent, std::move(light_component));
			}
		}

		void Scene::LoadAnimations(const easygltf::FGLTFRoot& root)
		{
			auto scene_root = CEntityManager::GetRoot();
			auto& root_component = CEntityManager::GetComponent<ecs::FRootComponent>(scene_root);

			for (auto& anim : root.animations)
			{
				ecs::FMeshAnimation animation;
				animation.name = anim.name;

				for (auto& samp : anim.samplers)
				{
					ecs::FAnimationSampler sampler{};

					if (samp.interpolation == "LINEAR")
						sampler.interpolation = ecs::EInterpolationType::eLinear;
					if (samp.interpolation == "STEP")
						sampler.interpolation = ecs::EInterpolationType::eStep;
					if (samp.interpolation == "CUBICSPLINE")
						sampler.interpolation = ecs::EInterpolationType::eCubicSpline;

					// Read sampler input time values
					{
						const easygltf::Accessor& accessor = root.accessors[samp.input];
						auto stream = root.getStream(accessor);

						// TODO: add other type support
						assert(accessor.componentType == easygltf::base::enumerators::component_type::float_type);

						f32* buf = new f32[accessor.count];
						util::Copy(buf, stream.data, stream.count * sizeof(f32));
						for (u64 index = 0; index < stream.count; index++)
							sampler.inputs.push_back(buf[index]);

						delete[] buf;

						for (auto input : sampler.inputs)
						{
							if (input < animation.start)
								animation.start = input;
							if (input > animation.end)
								animation.end = input;
						}
					}

					// Read sampler output T/R/S values 
					{
						const easygltf::Accessor& accessor = root.accessors[samp.output];
						auto stream = root.getStream(accessor);

						assert(accessor.componentType == easygltf::base::enumerators::component_type::float_type);

						if (accessor.type == easygltf::base::enumerators::accessor_type::vec3_type)
						{
							glm::vec3* buf = new glm::vec3[stream.count];
							util::Copy(buf, stream.data, stream.count * sizeof(glm::vec3));
							for (u64 index = 0; index < stream.count; index++)
								sampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));

							delete[] buf;
						}
						else if (accessor.type == easygltf::base::enumerators::accessor_type::vec4_type)
						{
							glm::vec4* buf = new glm::vec4[stream.count];
							util::Copy(buf, stream.data, stream.count * sizeof(glm::vec4));
							for (u64 index = 0; index < stream.count; index++)
								sampler.outputsVec4.push_back(buf[index]);

							delete[] buf;
						}
					}

					animation.samplers.push_back(sampler);
				}

				// Load animation channdls
				for (auto& source : anim.channels)
				{
					ecs::FAnimationChannel channel{};

					if (source.target.path == "rotation")
						channel.path = ecs::EPathType::eRotation;
					if (source.target.path == "translation")
						channel.path = ecs::EPathType::eTranslation;
					if (source.target.path == "scale")
						channel.path = ecs::EPathType::eScale;

					if (source.target.path == "weights") {
						log_warning("[Scene] Weights channel is not yet supported. Skipping.");
						continue;
					}
					channel.samplerIndex = source.sampler;

					auto target = node_to_entity.find(source.target.node);
					if (target != node_to_entity.end())
						channel.node = target->second;
					else
						continue;

					animation.channels.push_back(channel);
				}

				root_component.animations.emplace_back(animation);
			}

			log_info("[Scene] Loaded {} animations.", root_component.animations.size());
		}

		void Scene::LoadSkins(const easygltf::FGLTFRoot& root)
		{
			auto scene_root = CEntityManager::GetRoot();
			auto& root_component = CEntityManager::GetComponent<ecs::FRootComponent>(scene_root);

			for (auto& source : root.skins)
			{
				ecs::FMeshSkin skin{};
				skin.name = source.name;

				// Find skeleton root node
				if (source.skeleton > -1)
				{
					auto target = node_to_entity.find(source.skeleton);
					if (target != node_to_entity.end())
						skin.root = target->second;
				}

				// Find joint nodes
				for (int jointIndex : source.joints)
				{
					auto target = node_to_entity.find(jointIndex);
					if (target != node_to_entity.end())
						skin.joints.emplace_back(target->second);
				}

				// Get inverse bind matrices from buffer
				if (source.inverseBindMatrices > -1)
				{
					const easygltf::Accessor& accessor = root.accessors[source.inverseBindMatrices];
					auto stream = root.getStream(accessor);
					skin.inverseBindMatrices.resize(stream.count);
					util::Copy(skin.inverseBindMatrices.data(), stream.data, stream.count * sizeof(glm::mat4));
				}

				root_component.skins.emplace_back(skin);
			}

			log_info("[Scene] Loaded {} skins (skeletons).", root_component.skins.size());
		}
	}
}