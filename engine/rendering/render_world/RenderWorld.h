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
			FCamera& GetRenderCamera(uint32_t camera_id) override;
			glm::mat4 const& GetCameraMatrix(uint32_t camera_id, uint32_t matrix_index) override;

			void UpdateRenderCamera(uint32_t camera_id, const glm::vec3& translation, const glm::quat& rotation) override;
			void UpdateRenderCamera(uint32_t camera_id, const glm::mat4& view, const glm::mat4& projection, uint32_t width, uint32_t height) override;

			void UpdateNode(uint32_t node_id, const glm::mat4& matrix) override;

			void AddGeometryDraw(DrawCommand& draw_command) override;

			void AddMaterial(const FMaterial& material) override;
			const FMaterial& GetMaterial(uint32_t index) const override;

			void AddLight(const FLightCommit& light_data) override;

			void AddShadow(const FCascadedShadowCommit& shadow_data) override;
			void AddShadow(const FOmniShadowCommit& shadow_data) override;
			void AddShadow(const FSpotShadowCommit& shadow_data) override;

			void BeginFrame(float delta_time) override;
			void EndFrame() override;

			gfx::IndirectDrawCommand GetDrawCommand() override;
		private:
			void UpdateCameraFrustum(uint32_t camera_id);
		private:
			IRenderer& renderer;

			std::vector<WorldResource> world_resources{};

			std::unique_ptr<DynamicBuffer<FSceneNode>> world_nodes{};
			uint32_t nodes_offset{ 0u };

			std::unique_ptr<DynamicBuffer<FCamera>> world_cameras{};
			uint32_t camera_offset{ 0u };

			std::unique_ptr<DynamicBuffer<glm::mat4>> world_matrices{};
			uint32_t matrices_offset{ 0u };

			std::unique_ptr<DynamicBuffer<glm::vec4>> world_frustum{};
			uint32_t frustum_offset{ 0u };

			std::unique_ptr<DynamicBuffer<FDrawIndexedIndirectData>> indirect_draw_commands{};
			uint32_t draw_count{ 0u };

			std::unique_ptr<DynamicBuffer<DrawDescription>> draw_descriptions{};

			std::unique_ptr<DynamicBuffer<FMesh>> world_meshes{};
			
			std::unique_ptr<DynamicBuffer<FMaterial>> draw_materials{};
			uint32_t material_count{ 0u };

			std::unique_ptr<DynamicBuffer<FLightCommit>> scene_lights{};
			uint32_t light_count{ 0u };

			std::unique_ptr<DynamicBuffer<FCascadedShadowCommit>> scene_cascaded_shadows{};
			uint32_t cascaded_shadow_count{ 0u };

			std::unique_ptr<DynamicBuffer<FOmniShadowCommit>> omni_shadows{};
			uint32_t omni_shadow_count{ 0u };

			std::unique_ptr<DynamicBuffer<FSpotShadowCommit>> spot_shadows{};
			uint32_t spot_shadow_count{ 0u };
		};
	}
}