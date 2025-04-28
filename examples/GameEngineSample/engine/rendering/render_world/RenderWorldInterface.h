#pragma once

#include <edge/gfx/gfx_structures.h>
#include "../../../assets/shaders/include/interop.h"

namespace engine::render {
	enum class WorldResourceType {
		eCamera,
		eNode,
		eMesh,
		eLight
	};

	struct WorldResource {
		WorldResourceType type{};
		uint32_t resource_id{};
	};

	struct DrawCommand {
		uint32_t node_index{};

		uint32_t vertex_buffers{};
		uint32_t material_index{};

		uint32_t first_index{};
		uint32_t index_count{};
		uint32_t first_vertex{};
		uint32_t vertex_count{};
	};

	class IRenderWorld {
	public:
		virtual ~IRenderWorld() = default;

		virtual uint32_t get_resource_id(uint32_t resource_index) const = 0;

		virtual std::tuple<uint32_t, interop::Camera&> create_camera() = 0;
		virtual interop::Camera& get_render_camera(uint32_t camera_id) = 0;
		virtual glm::mat4 const& get_camera_matrix(uint32_t camera_id, uint32_t matrix_index) = 0;
		virtual uint32_t get_camera_count() const = 0;

		virtual void update_render_camera(uint32_t camera_id, const glm::vec3& translation, const glm::quat& rotation) = 0;
		virtual void update_render_camera(uint32_t camera_id, const glm::mat4& view, const glm::mat4& projection, bool perspective, uint32_t width, uint32_t height) = 0;

		virtual glm::mat4 const& get_matrix(uint32_t matrix_offset) const = 0;

		virtual void add_geometry_draw(uint32_t camera_id, uint32_t mesh_id) = 0;

		virtual uint32_t add_material(const interop::Material& material) = 0;
		virtual const interop::Material& get_material(uint32_t index) const = 0;

		virtual std::tuple<uint32_t, interop::Mesh&> create_mesh(uint32_t joint_count = 0u) = 0;
		virtual void update_mesh_matrices(uint32_t mesh_id, const glm::mat4& matrix) = 0;
		virtual void update_joint_matrices(uint32_t mesh_id, std::span<const glm::mat4> joints) = 0;
		virtual void attach_submesh(uint32_t mesh_id, const interop::SubMesh& submesh) = 0;
		virtual const interop::Mesh& get_mesh(uint32_t index) = 0;

		virtual void add_light(const interop::LightCommit& light_data) = 0;

		virtual void add_shadow(const interop::CascadedShadowCommit& shadow_data) = 0;
		virtual void add_shadow(const interop::OmniShadowCommit& shadow_data) = 0;
		virtual void add_shadow(const interop::SpotShadowCommit& shadow_data) = 0;

		virtual void begin_frame(float delta_time) = 0;
		virtual void end_frame() = 0;

		virtual edge::gfx::IBuffer* get_draw_buffer() = 0;
		virtual uint32_t get_draw_count() const = 0;

		virtual uint32_t get_mesh_count() const = 0;
	};
}