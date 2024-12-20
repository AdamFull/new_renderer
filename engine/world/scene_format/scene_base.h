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
			constexpr const uint32_t version{ 0x01010001 };
			constexpr const int32_t invalid_index{ -1 };
		}

		using node_flags_t = uint32_t;
		namespace node_flags
		{
			constexpr const uint32_t enabled{ 1 << 0 };
			constexpr const uint32_t visible{ 1 << 1 };
		}

		using component_type_t = uint32_t;
		namespace component_type
		{
			constexpr const uint32_t invalid{ 0u };
			constexpr const uint32_t transform{ 1u };
			constexpr const uint32_t camera{ 2u };
			constexpr const uint32_t mesh{ 3u };
			constexpr const uint32_t directional_light{ 4u };
			constexpr const uint32_t point_light{ 5u };
			constexpr const uint32_t spot_light{ 6u };
		}

		using primitive_topology_t = uint32_t;
		namespace primitive_topology
		{
			constexpr const uint32_t invalid{ 0u };
			constexpr const uint32_t points{ 1u };
			constexpr const uint32_t line{ 2u };
			constexpr const uint32_t line_loop{ 3u };
			constexpr const uint32_t line_strip{ 4u };
			constexpr const uint32_t triangles{ 5u };
			constexpr const uint32_t triangles_strip{ 6u };
			constexpr const uint32_t triangles_fan{ 7u };
		}

		using attribute_type_t = uint32_t;
		namespace attribute_type
		{
			constexpr const uint32_t invalid{ 0u };
			constexpr const uint32_t position{ 1u };
			constexpr const uint32_t normal{ 2u };
			constexpr const uint32_t tex_coord_0{ 3u };
			constexpr const uint32_t tex_coord_1{ 4u };
			constexpr const uint32_t color_0{ 5u };
			constexpr const uint32_t color_1{ 6u };
			constexpr const uint32_t joints{ 7u };
			constexpr const uint32_t weights{ 8u };
			constexpr const uint32_t custom{ 9u };
		}

		using buffer_view_type_t = uint32_t;
		namespace buffer_view_type
		{
			constexpr const uint32_t invalid{ 0u };
			constexpr const uint32_t array_buffer{ 1u };
			constexpr const uint32_t element_array_buffer{ 2u };
		}

		using light_flags_t = uint32_t;
		namespace light_flags
		{
			constexpr const uint32_t cast_shadows{ 1 << 0 };
		}

		using primitive_flags_t = uint32_t;
		namespace primitive_flags
		{
			constexpr const uint32_t cast_shadows{ 1 << 0 };
		}

		using accessor_id_t = int32_t;
		using buffer_id_t = int32_t;
		using buffer_view_id_t = int32_t;
		using component_id_t = int32_t;
		using material_id_t = int32_t;
		using scene_id_t = int32_t;

		struct FVersion
		{
			union {
				uint32_t code;
				struct {
					uint8_t major;
					uint8_t minor;
					uint16_t patch;
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
			std::unordered_map<attribute_type_t, accessor_id_t> attributes; // (required)
			std::vector<std::unordered_map<attribute_type_t, accessor_id_t>> targets;

			bool operator==(const FMeshPrimitiveLod& other) const
			{
				return this->attributes == other.attributes && this->targets == other.targets;
			}
		};

		struct FMeshPrimitive
		{
			std::vector<FMeshPrimitiveLod> lods{};

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
			std::vector<FMeshPrimitive> primitives; // (required)
			std::vector<float> weights;

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

			std::vector<FComponentDescription> components{};
		};
		
		// Base structure with scene data
		struct FSceneData
		{
			FHeader asset{};

			std::vector<FAudioEmitterComponent> audio_emitter_components{};
			std::vector<FMeshComponent> mesh_components{};
			std::vector<FTransformComponent> transform_components{};
			std::vector<FLightComponent> light_components{};

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
			uint64_t byte_offset{ 0ull };
			uint64_t byte_length{ 0ull }; // (required)
			uint64_t byte_stride{ 0ull };

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
			uint64_t byte_offset{ 0ull };
			uint64_t count{ 0ull }; // (required)
			bool normalized{ false };
			component_type_t component_type{ component_type::invalid }; // (required)
			std::string type{ }; // (required)

			std::vector<double> min{};
			std::vector<double> max{};

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
			std::vector<FAccessor> accessors{};
			std::vector<FBufferView> buffer_views{};
			std::vector<FBuffer> buffers{};
			// animations
			// skins
			// skeleton (contains data about skin and bones/nodes)
		};

		//struct FGLTFRoot : public Default
		//{
		//	Stream getStream(const Accessor& accessor) const;
		//	Stream getStream(int32_t accessor_id) const;
		//
		//	std::vector<Accessor> accessors{};
		//	std::vector<Animation> animations{};
		//	Asset asset{};
		//	std::vector<Buffer> buffers{};
		//	std::vector<BufferView> bufferViews{};
		//	std::vector<Camera> cameras{};
		//	std::vector<Image> images{};
		//	std::vector<Material> materials{};
		//	std::vector<Mesh> meshes{};
		//	std::vector<Node> nodes{};
		//	std::vector<Sampler> samplers{};
		//	std::vector<Scene> scenes{};
		//	std::vector<Skin> skins{};
		//	std::vector<Texture> textures{};
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