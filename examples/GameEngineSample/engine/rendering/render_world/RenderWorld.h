#pragma once

#include "RenderWorldInterface.h"
#include "../DynamicBuffer.h"

#include <memory>
#include <vector>

namespace engine::render {
	class IRenderer;

	class RenderWorld final : public IRenderWorld {
	public:
		RenderWorld(IRenderer& renderer);
		~RenderWorld() = default;

		uint32_t get_resource_id(uint32_t resource_index) const;

		std::tuple<uint32_t, interop::Camera&> create_camera() override;
		interop::Camera& get_render_camera(uint32_t camera_id) override;
		glm::mat4 const& get_camera_matrix(uint32_t camera_id, uint32_t matrix_index) override;
		uint32_t get_camera_count() const override;

		void update_render_camera(uint32_t camera_id, const glm::vec3& translation, const glm::quat& rotation) override;
		void update_render_camera(uint32_t camera_id, const glm::mat4& view, const glm::mat4& projection, bool perspective, uint32_t width, uint32_t height) override;

		glm::mat4 const& get_matrix(uint32_t matrix_offset) const override;

		void add_geometry_draw(uint32_t camera_id, uint32_t mesh_id) override;

		uint32_t add_material(const interop::Material& material) override;
		const interop::Material& get_material(uint32_t index) const override;

		//interop::Mesh& BeginUpdateMesh(uint32_t index);
		//void EndUpdateMesh(uint32_t index);
		std::tuple<uint32_t, interop::Mesh&> create_mesh(uint32_t joint_count) override;
		void update_mesh_matrices(uint32_t mesh_id, const glm::mat4& matrix) override;
		void update_joint_matrices(uint32_t mesh_id, std::span<const glm::mat4> joints) override;
		void add_submeshes(std::span<const interop::SubMesh> submeshes) override;
		const interop::Mesh& get_mesh(uint32_t index) override;

		void add_light(const interop::LightCommit& light_data) override;

		void add_shadow(const interop::CascadedShadowCommit& shadow_data) override;
		void add_shadow(const interop::OmniShadowCommit& shadow_data) override;
		void add_shadow(const interop::SpotShadowCommit& shadow_data) override;

		void begin_frame(float delta_time) override;
		void end_frame() override;

		edge::gfx::IBuffer* get_draw_buffer() override;
		uint32_t get_draw_count() const override;

		uint32_t get_mesh_count() const override;
	private:
		void update_camera_frustum(uint32_t camera_id);
	private:
		IRenderer& renderer;

		uint32_t frame_counter{ 0u };

		std::vector<WorldResource> world_resources{};

		std::unique_ptr<DynamicBuffer<interop::Camera>> world_cameras{};
		uint32_t camera_offset{ 0u };

		std::unique_ptr<DynamicBuffer<glm::mat4>> world_matrices{};
		uint32_t matrices_offset{ 0u };

		std::unique_ptr<DynamicBuffer<glm::vec4>> world_frustum{};
		uint32_t frustum_offset{ 0u };

		uint32_t draw_count{ 0u };

		std::unique_ptr<DynamicBuffer<interop::DrawDescription>> draw_descriptions{};

		std::unique_ptr<DynamicBuffer<interop::Mesh>> world_meshes{};
		uint32_t mesh_count{ 0u };

		std::unique_ptr<DynamicBuffer<interop::SubMesh>> world_submeshes{};
		uint32_t submesh_count{ 0u };

		std::unique_ptr<DynamicBuffer<interop::Material>> draw_materials{};
		uint32_t material_count{ 0u };

		std::unique_ptr<DynamicBuffer<interop::LightCommit>> scene_lights{};
		uint32_t light_count{ 0u };

		std::unique_ptr<DynamicBuffer<interop::CascadedShadowCommit>> scene_cascaded_shadows{};
		uint32_t cascaded_shadow_count{ 0u };

		std::unique_ptr<DynamicBuffer<interop::OmniShadowCommit>> omni_shadows{};
		uint32_t omni_shadow_count{ 0u };

		std::unique_ptr<DynamicBuffer<interop::SpotShadowCommit>> spot_shadows{};
		uint32_t spot_shadow_count{ 0u };
	};
}