#pragma once

#include <engine/rendering/render_world/RenderWorldInterface.h>
#include <engine/rendering/DynamicBuffer.h>

#include <memory>
#include <vector>

namespace engine
{
	namespace render
	{
		class IRenderer;

		class RenderWorld final : public IRenderWorld
		{
		public:
			RenderWorld(IRenderer& renderer);
			~RenderWorld() = default;

			uint32_t CreateRenderCamera() override;
			shlib::Camera& GetRenderCamera(uint32_t camera_id) override;
			glm::mat4 const& GetCameraMatrix(uint32_t camera_id, uint32_t matrix_index) override;
			uint32_t GetCameraId(uint32_t camera_id) const override;
			uint32_t GetCameraCount() const override;

			void UpdateRenderCamera(uint32_t camera_id, const glm::vec3& translation, const glm::quat& rotation) override;
			void UpdateRenderCamera(uint32_t camera_id, const glm::mat4& view, const glm::mat4& projection, uint32_t width, uint32_t height) override;

			glm::mat4 const& GetMatrix(uint32_t matrix_offset) const override;

			uint32_t CreateNode() override;
			void UpdateNode(uint32_t node_id, const glm::mat4& matrix) override;
			glm::mat4 const& GetNodeMatrix(uint32_t node_id, uint32_t matrix_offset) const override;
			const shlib::SceneNode& GetNode(uint32_t node_id) const override;
			uint32_t GetNodeId(uint32_t resource_id) const override;

			void AddGeometryDraw(const shlib::DrawDescription& draw_command) override;

			uint32_t AddMaterial(const shlib::Material& material) override;
			const shlib::Material& GetMaterial(uint32_t index) const override;

			//shlib::Mesh& BeginUpdateMesh(uint32_t index);
			//void EndUpdateMesh(uint32_t index);
			uint32_t AddMesh(const shlib::Mesh& mesh) override;
			const shlib::Mesh& GetMesh(uint32_t index) override;

			void AddLight(const shlib::LightCommit& light_data) override;

			void AddShadow(const shlib::CascadedShadowCommit& shadow_data) override;
			void AddShadow(const shlib::OmniShadowCommit& shadow_data) override;
			void AddShadow(const shlib::SpotShadowCommit& shadow_data) override;

			void BeginFrame(float delta_time) override;
			void EndFrame() override;

			gfx::Buffer* GetDrawBuffer() override;
			uint32_t GetDrawCount() const override;
		private:
			void UpdateCameraFrustum(uint32_t camera_id);
		private:
			IRenderer& renderer;

			std::vector<WorldResource> world_resources{};

			std::unique_ptr<DynamicBuffer<shlib::SceneNode>> world_nodes{};
			uint32_t nodes_offset{ 0u };

			std::unique_ptr<DynamicBuffer<shlib::Camera>> world_cameras{};
			uint32_t camera_offset{ 0u };

			std::unique_ptr<DynamicBuffer<glm::mat4>> world_matrices{};
			uint32_t matrices_offset{ 0u };

			std::unique_ptr<DynamicBuffer<glm::vec4>> world_frustum{};
			uint32_t frustum_offset{ 0u };

			uint32_t draw_count{ 0u };

			std::unique_ptr<DynamicBuffer<shlib::DrawDescription>> draw_descriptions{};

			std::unique_ptr<DynamicBuffer<shlib::Mesh>> world_meshes{};
			uint32_t mesh_count{ 0u };
			
			std::unique_ptr<DynamicBuffer<shlib::Material>> draw_materials{};
			uint32_t material_count{ 0u };

			std::unique_ptr<DynamicBuffer<shlib::LightCommit>> scene_lights{};
			uint32_t light_count{ 0u };

			std::unique_ptr<DynamicBuffer<shlib::CascadedShadowCommit>> scene_cascaded_shadows{};
			uint32_t cascaded_shadow_count{ 0u };

			std::unique_ptr<DynamicBuffer<shlib::OmniShadowCommit>> omni_shadows{};
			uint32_t omni_shadow_count{ 0u };

			std::unique_ptr<DynamicBuffer<shlib::SpotShadowCommit>> spot_shadows{};
			uint32_t spot_shadow_count{ 0u };
		};
	}
}