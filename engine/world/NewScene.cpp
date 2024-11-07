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
		render::VertexAttributeFlag GetVertexAttributeFlag(const std::string& name)
		{
			using namespace engine::render;

			if (name == easygltf::base::enumerators::vertex_attribute_type::position) return render::VertexAttributeFlag::ePosition;
			else if (name == easygltf::base::enumerators::vertex_attribute_type::color_0) return render::VertexAttributeFlag::eColor;
			else if (name == easygltf::base::enumerators::vertex_attribute_type::texcoord_0) return render::VertexAttributeFlag::eTexCoord;
			else if (name == easygltf::base::enumerators::vertex_attribute_type::normal) return render::VertexAttributeFlag::eNormal;
			else if (name == easygltf::base::enumerators::vertex_attribute_type::tangent) return render::VertexAttributeFlag::eTangent;
			else if (name == easygltf::base::enumerators::vertex_attribute_type::joints_0) return render::VertexAttributeFlag::eJoint;
			else if (name == easygltf::base::enumerators::vertex_attribute_type::weights_0) return render::VertexAttributeFlag::eWeight;

			return render::VertexAttributeFlag::ePosition;
		}

		gfx::Format GetVertexFormat(const easygltf::Accessor& accessor)
		{
			if (accessor.type == easygltf::base::enumerators::accessor_type::vec4_type)
			{
				switch (accessor.componentType)
				{
				case easygltf::base::enumerators::component_type::byte_type: return gfx::Format::eR8G8B8A8Sint; break;
				case easygltf::base::enumerators::component_type::ubyte_type: return gfx::Format::eR8G8B8A8Unorm; break;
				case easygltf::base::enumerators::component_type::short_type: return gfx::Format::eR16G16B16A16Sint; break;
				case easygltf::base::enumerators::component_type::ushort_type: return gfx::Format::eR16G16B16A16Unorm; break;
				case easygltf::base::enumerators::component_type::int_type: return gfx::Format::eR32G32B32A32Sint; break;
				case easygltf::base::enumerators::component_type::uint_type: return gfx::Format::eR32G32B32A32Uint; break;
				case easygltf::base::enumerators::component_type::float_type: return gfx::Format::eR32G32B32A32Sfloat; break;
				case easygltf::base::enumerators::component_type::double_type: return gfx::Format::eR64G64B64A64Sfloat; break;
				default:
					break;
				}
			}
			else if (accessor.type == easygltf::base::enumerators::accessor_type::vec3_type)
			{
				switch (accessor.componentType)
				{
				case easygltf::base::enumerators::component_type::byte_type: return gfx::Format::eR8G8B8Sint; break;
				case easygltf::base::enumerators::component_type::ubyte_type: return gfx::Format::eR8G8B8Unorm; break;
				case easygltf::base::enumerators::component_type::short_type: return gfx::Format::eR16G16B16Sint; break;
				case easygltf::base::enumerators::component_type::ushort_type: return gfx::Format::eR16G16B16Unorm; break;
				case easygltf::base::enumerators::component_type::int_type: return gfx::Format::eR32G32B32Sint; break;
				case easygltf::base::enumerators::component_type::uint_type: return gfx::Format::eR32G32B32Uint; break;
				case easygltf::base::enumerators::component_type::float_type: return gfx::Format::eR32G32B32Sfloat; break;
				case easygltf::base::enumerators::component_type::double_type: return gfx::Format::eR64G64B64Sfloat; break;
				default:
					break;
				}
			}
			else if (accessor.type == easygltf::base::enumerators::accessor_type::vec2_type)
			{
				switch (accessor.componentType)
				{
				case easygltf::base::enumerators::component_type::byte_type: return gfx::Format::eR8G8Sint; break;
				case easygltf::base::enumerators::component_type::ubyte_type: return gfx::Format::eR8G8Unorm; break;
				case easygltf::base::enumerators::component_type::short_type: return gfx::Format::eR16G16Sint; break;
				case easygltf::base::enumerators::component_type::ushort_type: return gfx::Format::eR16G16Unorm; break;
				case easygltf::base::enumerators::component_type::int_type: return gfx::Format::eR32G32Sint; break;
				case easygltf::base::enumerators::component_type::uint_type: return gfx::Format::eR32G32Uint; break;
				case easygltf::base::enumerators::component_type::float_type: return gfx::Format::eR32G32Sfloat; break;
				case easygltf::base::enumerators::component_type::double_type: return gfx::Format::eR64G64Sfloat; break;
				default:
					break;
				}
			}
			else if (accessor.type == easygltf::base::enumerators::accessor_type::scalar_type)
			{
				switch (accessor.componentType)
				{
				case easygltf::base::enumerators::component_type::byte_type: return gfx::Format::eR8Sint; break;
				case easygltf::base::enumerators::component_type::ubyte_type: return gfx::Format::eR8Unorm; break;
				case easygltf::base::enumerators::component_type::short_type: return gfx::Format::eR16Sint; break;
				case easygltf::base::enumerators::component_type::ushort_type: return gfx::Format::eR16Unorm; break;
				case easygltf::base::enumerators::component_type::int_type: return gfx::Format::eR32Sint; break;
				case easygltf::base::enumerators::component_type::uint_type: return gfx::Format::eR32Uint; break;
				case easygltf::base::enumerators::component_type::float_type: return gfx::Format::eR32Sfloat; break;
				case easygltf::base::enumerators::component_type::double_type: return gfx::Format::eR64Sfloat; break;
				default:
					break;
				}
			}

			return gfx::Format::eUndefined;
		}

		gfx::IndexType GetIndexType(const easygltf::Accessor& accessor)
		{
			switch (accessor.componentType)
			{
			case easygltf::base::enumerators::component_type::ushort_type: return gfx::IndexType::eUint16; break;
			case easygltf::base::enumerators::component_type::uint_type: return gfx::IndexType::eUint32; break;
			default:
				break;
			}

			return gfx::IndexType::eUint32;
		}

		render::GeometryBufferCreateInfo GetGeometryBufferCreateInfo(const easygltf::FGLTFRoot& root, const easygltf::Primitive& primitive)
		{
			render::GeometryBufferCreateInfo create_info{};

			for (const auto& [name, accessor_index] : primitive.attributes)
			{
				// NOT SUPPORTED FOR NOW
				if (name == "TEXCOORD_1")
					continue;

				const easygltf::Accessor& accessor = root.accessors[accessor_index];

				auto& new_description = create_info.attribute_descriptions.emplace_back();
				new_description.attribute_flag = GetVertexAttributeFlag(name);
				new_description.format = GetVertexFormat(accessor);
			}

			if (primitive.indices >= 0)
			{
				const easygltf::Accessor& accessor = root.accessors[primitive.indices];
				create_info.index_description.type = GetIndexType(accessor);
			}

			std::sort(create_info.attribute_descriptions.begin(), create_info.attribute_descriptions.end(), [](const auto& lhs, const auto& rhs) { return utl::enum_cast(lhs.attribute_flag) < utl::enum_cast(rhs.attribute_flag); });

			return create_info;
		}

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
			EntityManager::RegisterSystem<ecs::CInputSystem>();
			EntityManager::RegisterSystem<ecs::CAnimationSystem>();
			EntityManager::RegisterSystem<ecs::CScenegraphSystem>();
			//EntityManager::RegisterSystem<ecs::CSkinningSystem>(render_interface);

			EntityManager::RegisterSystem<ecs::CCameraSystem>(renderer);
			//EntityManager::RegisterSystem<ecs::CShadowsSystem>(render_interface);
			EntityManager::RegisterSystem<ecs::CLightingSystem>(renderer);
			EntityManager::RegisterSystem<ecs::CMeshSystem>(renderer);
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

		void Scene::Update(float delta_time)
		{
			EntityManager::Update(delta_time);
		}

		void Scene::LoadGLTF(easygltf::FGLTFRoot& root)
		{
			auto& resource_context = renderer.GetResourceContext();
			// Load textures
			for (const auto& texture : root.textures)
			{
				i32 texture_source{ texture.source };
				if (texture.source < 0)
					texture_source = texture.khrTextureBasisU.source;

				auto& image = root.images[texture_source];
				auto& sampler = root.samplers[texture.sampler];

				auto sampler_create_info = GetSamplerCreateInfo(sampler);
				gfx::Sampler* texture_sampler = resource_context.GetSampler(sampler_create_info);

				// TODO: add ability to override format and sampler params
				// TODO: find format from materials
				auto image_path = filesystem::fs::path_append(root.rootDirectory, easygltf::url_decode(image.uri));
				resources.textures.push_back(renderer.AddRenderResource(image_path, texture_sampler));
			}

			for (u64 idx = 0ull; idx < root.materials.size(); ++idx)
			{
				auto& material = root.materials[idx];
				render::FMaterial render_material{};

				// TODO: add my own ext that will contain shader EXT_material_shader or something. But in my case it will contain name in format {library}:{shader_name} and other helpfull data

				// Attach base color (aka albedo)
				if (auto& texture = material.pbrMetallicRoughness.baseColorTexture; texture.index >= 0)
					render_material.baseColorTexture = resources.textures[texture.index];

				// Attach metallic roughness texture (metallic = green, roughness = blue)
				if (auto& texture = material.pbrMetallicRoughness.metallicRoughnessTexture; texture.index >= 0)
					render_material.metallicRoughnessTexture = resources.textures[texture.index];

				// Attach normal map texture
				if (auto& texture = material.normalTexture; texture.index >= 0)
				{
					render_material.normalScale = utl::cast<f32>(texture.scale);
					render_material.normalTexture = resources.textures[texture.index];
				}

				// Attach baked ambient occlusion texture
				if (auto& texture = material.occlusionTexture; texture.index >= 0)
				{
					render_material.occlusionStrenth = utl::cast<f32>(texture.strength);
					render_material.occlusionTexture = resources.textures[texture.index];
				}

				// Attach emissive texture
				if (auto& texture = material.emissiveTexture; texture.index >= 0)
					render_material.emissiveTexture = resources.textures[texture.index];

				render_material.emissiveFactor = glm::make_vec3(material.emissiveFactor.data());
				render_material.roughnessFactor = utl::cast<f32>(material.pbrMetallicRoughness.roughnessFactor);
				render_material.metallicFactor = utl::cast<f32>(material.pbrMetallicRoughness.metallicFactor);
				render_material.baseColorFactor = glm::make_vec4(material.pbrMetallicRoughness.baseColorFactor.data());

				if (material.alphaMode == easygltf::base::enumerators::alpha_mode::blend)
					render_material.alphaMode = utl::cast<u32>(render::EAlphaMode::eBlend);
				else if (material.alphaMode == easygltf::base::enumerators::alpha_mode::mask)
					render_material.alphaMode = utl::cast<u32>(render::EAlphaMode::eMask);
				else
					render_material.alphaMode = utl::cast<u32>(render::EAlphaMode::eOpaque);

				render_material.alphaCutoff = utl::cast<f32>(material.alphaCutoff);
				render_material.doubleSided = material.doubleSided;

				renderer.AddMaterial(render_material);
			}

			// Loading nodes
			const easygltf::Scene& scene = root.scenes[root.scene > -1 ? root.scene : 0];
			auto scene_root = EntityManager::CreateRoot(scene.name);

			for (const auto& node_index : scene.nodes)
				LoadNode(root, scene_root, node_index);

			LoadAnimations(root);
			LoadSkins(root);

			EntityManager::Create();
		}

		void Scene::LoadNode(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index, f32 global_scale)
		{
			const auto& node = root.nodes[node_index];

			auto new_node = EntityManager::CreateNode(node.name);

			log_info("[Scene] Loadind node: {}:{}.", node.name, node_index);

			node_to_entity[node_index] = new_node;

			// Parse node local transform
			auto& transform = EntityManager::GetComponent<ecs::FTransformComponent>(new_node);
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

			EntityManager::AttachChild(parent, new_node);
		}

		void Scene::LoadMeshComponent(const easygltf::FGLTFRoot& root, const entt::entity parent, u32 node_index)
		{
			const auto& node = root.nodes[node_index];
			const auto& mesh = root.meshes[node.mesh];

			auto& mesh_component = EntityManager::AddComponent<ecs::FMeshComponent>(parent, ecs::FMeshComponent{});
			mesh_component.skin = node.skin;

			// Load mesh primitives
			for (u64 i = 0ull; i < mesh.primitives.size(); ++i)
			{
				const auto& primitive = mesh.primitives[i];

				// TODO: add min max
				glm::vec3 pos_min{}, pos_max{};

				// Load vertices
				render::GeometryBufferCreateInfo geometry_ci = GetGeometryBufferCreateInfo(root, primitive);
				auto& geometry_buffer = renderer.RequestDeinterleavedGeometryBuffer(geometry_ci);

				uint32_t stream_count{ 0u };
				std::array<render::AttributeStream, render::k_max_attribure_description> attribute_streams{};
				for (const auto& [name, accessor_index] : primitive.attributes)
				{
					// NOT SUPPORTED FOR NOW
					if (name == "TEXCOORD_1")
						continue;

					const easygltf::Accessor& accessor = root.accessors[accessor_index];
					auto stream = root.getStream(accessor);

					auto attribute_type = GetVertexAttributeFlag(name);

					auto& attrib_stream = attribute_streams[stream_count++];
					attrib_stream.attribute_flag = attribute_type;
					attrib_stream.byte_range = std::span<const uint8_t>(stream.data, stream.count * stream.stride);
					attrib_stream.count = stream.count;

					if (attribute_type == render::VertexAttributeFlag::ePosition)
					{
						pos_min = !accessor.min.empty() ? glm::vec3(accessor.min[0], accessor.min[1], accessor.min[2]) : glm::vec3(0.f);
						pos_max = !accessor.max.empty() ? glm::vec3(accessor.max[0], accessor.max[1], accessor.max[2]) : glm::vec3(0.f);
					}
				}

				render::IndexStream index_stream{};
				if (primitive.indices >= 0)
				{
					const easygltf::Accessor& accessor = root.accessors[primitive.indices];
					auto stream = root.getStream(accessor);

					index_stream.byte_range = std::span<const uint8_t>(stream.data, stream.count * stream.stride);
					index_stream.count = stream.count;
				}

				// Create mesh primitive
				ecs::FMeshPrimitive mesh_primitive{};
				mesh_primitive.vbo = Hash(geometry_ci);
				mesh_primitive.setDimensions(pos_min, pos_max);
				mesh_primitive.vertex_range = geometry_buffer.AddMesh(attribute_streams, index_stream);

				// NOTE: Updating material Create info for future shader creation
				if (primitive.material >= 0)
					mesh_primitive.material = primitive.material;
				
				mesh_component.primitives.emplace_back(mesh_primitive);
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

			EntityManager::AddComponent<ecs::FCameraComponent>(parent, std::move(camera_component));
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
				EntityManager::AddComponent<ecs::FDirectionalLightComponent>(parent, std::move(light_component));
			}
			else if (light.type == easygltf::extensions::KHR_lights_punctual::enumerators::light_type::spot)
			{
				ecs::FSpotLightComponent light_component;
				light_component.color = glm::make_vec3(light.color.data());
				light_component.innerAngle = light.spot.innerConeAngle;
				light_component.outerAngle = light.spot.outerConeAngle;
				light_component.intencity = light.intensity;
				EntityManager::AddComponent<ecs::FSpotLightComponent>(parent, std::move(light_component));
			}
			else if (light.type == easygltf::extensions::KHR_lights_punctual::enumerators::light_type::point)
			{
				ecs::FPointLightComponent light_component;
				light_component.color = glm::make_vec3(light.color.data());
				light_component.intencity = light.intensity;
				light_component.radius = (std::max)(light.range, 1.0);
				EntityManager::AddComponent<ecs::FPointLightComponent>(parent, std::move(light_component));
			}
		}

		void Scene::LoadAnimations(const easygltf::FGLTFRoot& root)
		{
			auto scene_root = EntityManager::GetRoot();
			auto& root_component = EntityManager::GetComponent<ecs::FRootComponent>(scene_root);

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
			auto scene_root = EntityManager::GetRoot();
			auto& root_component = EntityManager::GetComponent<ecs::FRootComponent>(scene_root);

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