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

			void UpdateNode(uint32_t node_id, const glm::mat4& matrix) override;

			void AddGeometryDraw(DrawCommand& draw_command) override;

			void AddMaterial(const FMaterial& material) override;

			void AddLight(const FLightCommit& light_data) override;

			void BeginFrame() override;
			void EndFrame() override;

			gfx::IndirectDrawCommand GetDrawCommand() override;
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
		};
	}
}