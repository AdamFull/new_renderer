#pragma once 

#include <standart_library.h>
#include <nlohmann/json.hpp>

#include <glm/glm.hpp>

namespace engine
{
	namespace world
	{
		namespace defaults
		{
			constexpr const u32 version{ 0x01010001 };
			constexpr const i32 invalid_index{ -1 };
		}

		using node_flags_t = u32;
		namespace node_flags
		{
			constexpr const u32 enabled{ 1 << 0 };
			constexpr const u32 visible{ 1 << 1 };
		}

		using component_type_t = u32;
		namespace component_type
		{
			constexpr const u32 invalid{ 0u };
			constexpr const u32 transform{ 1u };
			constexpr const u32 camera{ 2u };
			constexpr const u32 mesh{ 3u };
			constexpr const u32 directional_light{ 4u };
			constexpr const u32 point_light{ 5u };
			constexpr const u32 spot_light{ 6u };
		}

		using primitive_topology_t = u32;
		namespace primitive_topology
		{
			constexpr const u32 invalid{ 0u };
			constexpr const u32 points{ 1u };
			constexpr const u32 line{ 2u };
			constexpr const u32 line_loop{ 3u };
			constexpr const u32 line_strip{ 4u };
			constexpr const u32 triangles{ 5u };
			constexpr const u32 triangles_strip{ 6u };
			constexpr const u32 triangles_fan{ 7u };
		}

		using attribute_type_t = u32;
		namespace attribute_type
		{
			constexpr const u32 invalid{ 0u };
			constexpr const u32 position{ 1u };
			constexpr const u32 normal{ 2u };
			constexpr const u32 tex_coord_0{ 3u };
			constexpr const u32 tex_coord_1{ 4u };
			constexpr const u32 color_0{ 5u };
			constexpr const u32 color_1{ 6u };
			constexpr const u32 joints{ 7u };
			constexpr const u32 weights{ 8u };
			constexpr const u32 custom{ 9u };
		}

		using buffer_view_type_t = u32;
		namespace buffer_view_type
		{
			constexpr const u32 invalid{ 0u };
			constexpr const u32 array_buffer{ 1u };
			constexpr const u32 element_array_buffer{ 2u };
		}

		using light_flags_t = u32;
		namespace light_flags
		{
			constexpr const u32 cast_shadows{ 1 << 0 };
		}

		using primitive_flags_t = u32;
		namespace primitive_flags
		{
			constexpr const u32 cast_shadows{ 1 << 0 };
		}

		using accessor_id_t = i32;
		using buffer_id_t = i32;
		using buffer_view_id_t = i32;
		using component_id_t = i32;
		using material_id_t = i32;
		using scene_id_t = i32;

		struct FVersion
		{
			union {
				u32 code;
				struct {
					u8 major;
					u8 minor;
					u16 patch;
				};
			};

			bool operator==(const FVersion& other) const
			{
				return this->code == other.code;
			}
		};

		struct FHeader
		{
			FVersion version{};
			std::string description;

			bool operator==(const FHeader& other) const
			{
				return this->version == other.version && this->description == other.description;
			}
		};

		struct FAudioEmitterComponent
		{
			std::string src; // (required)

			bool operator==(const FAudioEmitterComponent& other) const
			{
				return this->src == other.src;
			}
		};

		struct FCameraComponent
		{

		};

		struct FMeshPrimitiveLod
		{
			utl::hash_map<attribute_type_t, accessor_id_t> attributes; // (required)
			utl::vector<utl::hash_map<attribute_type_t, accessor_id_t>> targets;

			bool operator==(const FMeshPrimitiveLod& other) const
			{
				return this->attributes == other.attributes && this->targets == other.targets;
			}
		};

		struct FMeshPrimitive
		{
			utl::vector<FMeshPrimitiveLod> lods{};

			material_id_t material{ defaults::invalid_index };
			accessor_id_t indices{ defaults::invalid_index };
			primitive_topology_t mode{ primitive_topology::invalid };

			primitive_flags_t flags{};

			bool operator==(const FMeshPrimitive& other) const
			{
				return
					this->lods == other.lods && this->material == other.material &&
					this->indices == other.indices && this->mode == other.mode &&
					this->flags == other.flags;
			}
		};

		struct FMeshComponent
		{
			std::string src; // (required) link to asset or asset uuid/guid
			utl::vector<FMeshPrimitive> primitives; // (required)
			utl::vector<float> weights;

			bool operator==(const FMeshComponent& other) const
			{
				return this->src == other.src && this->primitives == other.primitives &&
					this->weights == other.weights;
			}
		};

		struct FTransformComponent
		{
			glm::vec3 position{};
			glm::quat rotation{};
			glm::vec3 scale{};

			glm::mat4 matrix{};

			bool operator==(const FTransformComponent& other) const
			{
				return this->position == other.position && this->rotation == other.rotation &&
					this->scale == other.scale && this->matrix == other.matrix;
			}
		};

		struct FLightComponent
		{
			glm::vec3 color{ 1.f };
			f32 intensity{ 1.f };
			f32 range{ 1.f };

			f32 innerAngle{};
			f32 outerAngle{};

			light_flags_t flags{};

			bool operator==(const FLightComponent& other) const
			{
				return this->color == other.color && this->intensity == other.intensity &&
					this->range == other.range && this->innerAngle == other.innerAngle &&
					this->outerAngle == other.outerAngle && this->flags == other.flags;
			}
		};

		struct FComponentDescription
		{
			component_type_t type{ component_type::invalid };
			component_id_t id{ defaults::invalid_index };
		};

		struct FNode
		{
			std::string name;
			node_flags_t flags{ node_flags::enabled | node_flags::visible };

			utl::vector<FComponentDescription> components{};
		};
		
		// Base structure with scene data
		struct FSceneData
		{
			FHeader asset{};

			utl::vector<FAudioEmitterComponent> audio_emitter_components{};
			utl::vector<FMeshComponent> mesh_components{};
			utl::vector<FTransformComponent> transform_components{};
			utl::vector<FLightComponent> light_components{};

			scene_id_t current_scene{};
		};


		struct FBuffer
		{
			std::string uri{};

			bool operator==(const FBuffer& other) const
			{
				return this->uri == other.uri;
			}
		};

		struct FBufferView
		{
			buffer_id_t buffer{ defaults::invalid_index }; // (required)
			u64 byte_offset{ 0ull };
			u64 byte_length{ 0ull }; // (required)
			u64 byte_stride{ 0ull };

			buffer_view_type_t type{ buffer_view_type::invalid };

			bool operator==(const FBufferView& other) const
			{
				return
					this->buffer == other.buffer && this->byte_offset == other.byte_offset && 
					this->byte_length == other.byte_length && this->byte_stride == other.byte_stride && 
					this->type == other.type;
			}
		};

		struct FAccessor
		{
			buffer_view_id_t buffer_view{ defaults::invalid_index };
			u64 byte_offset{ 0ull };
			u64 count{ 0ull }; // (required)
			bool normalized{ false };
			component_type_t component_type{ component_type::invalid }; // (required)
			std::string type{ }; // (required)

			utl::vector<double> min{};
			utl::vector<double> max{};

			bool operator==(const FAccessor& other) const
			{
				return
					this->buffer_view == other.buffer_view && this->byte_offset == other.byte_offset && 
					this->count == other.count && this->normalized == other.normalized && 
					this->component_type == other.component_type && this->type == other.type &&
					this->min == other.min && this->max == other.max;
			}
		};

		struct FAsset
		{

		};

		struct FAudioAsset : FAsset
		{

		};

		struct FMaterialAsset : FAsset
		{

		};

		struct FMeshAsset : FAsset
		{
			utl::vector<FAccessor> accessors{};
			utl::vector<FBufferView> buffer_views{};
			utl::vector<FBuffer> buffers{};
			// animations
			// skins
			// skeleton (contains data about skin and bones/nodes)
		};

		//struct FGLTFRoot : public Default
		//{
		//	Stream getStream(const Accessor& accessor) const;
		//	Stream getStream(int32_t accessor_id) const;
		//
		//	utl::vector<Accessor> accessors{};
		//	utl::vector<Animation> animations{};
		//	Asset asset{};
		//	utl::vector<Buffer> buffers{};
		//	utl::vector<BufferView> bufferViews{};
		//	utl::vector<Camera> cameras{};
		//	utl::vector<Image> images{};
		//	utl::vector<Material> materials{};
		//	utl::vector<Mesh> meshes{};
		//	utl::vector<Node> nodes{};
		//	utl::vector<Sampler> samplers{};
		//	utl::vector<Scene> scenes{};
		//	utl::vector<Skin> skins{};
		//	utl::vector<Texture> textures{};
		//
		//	// Extensions
		//	extensions::KHR_audio::Audio khrAudio{};
		//	extensions::KHR_lights_punctual::LightsPunctual khrLightsPunctual{};
		//	extensions::KHR_materials_variants::MaterialVariants khrMaterialVariants{};
		//
		//	int32_t scene{ -1 }; // The index of the default scene.
		//
		//	mutable std::unordered_set<std::string> extensionsUsed;
		//	mutable std::unordered_set<std::string> extensionsRequired;
		//
		//	std::string rootDirectory{};
		//};
	}
}