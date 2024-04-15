#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CRenderWorld : public IRenderWorld
		{
		public:
			CRenderWorld(IRenderInterface* render_interface);
			~CRenderWorld() = default;

			void create() override;

			u64 registerFrustum(ERenderGeometryCullerType type) override;
			void updateFrustum(u64 frustum_id, const glm::mat4& view, const glm::mat4& projection) override;

			void addDrawGeometry(const std::vector<FGeometryRawBatch>& new_geometry) override;

			void render(f32 dt) override;

			void addLight(const FDirectionalLightCommit& light) override;
			void addLight(const FPointLightCommit& light) override;
			void addLight(const FSpotLightCommit& light) override;

			void addShadow(const FCascadedShadowCommit& commit) override;
			void addShadow(const FOmniShadowCommit& commit) override;
			void addShadow(const FSpotShadowCommit& commit) override;

			void updateNode(u32 node_id, const FSceneNode& node) override;
			void updateSkin(const std::vector<glm::mat4>& matrices) override;

			void addMaterial(const FMaterial& material) override;
			void addMaterials(const std::vector<FMaterial>& materials) override;
			const FMaterial& getMaterial(u64 index) const override;

			void addMesh(const FMesh& mesh) override;
			void addMeshes(const std::vector<FMesh>& meshes) override;

			const std::unique_ptr<IGPUVector>& getDrawBuffer(ERenderBufferIndices buffer_index) override;
			const std::vector<FGeometryCPUBatch>& getGeometryBatches() const override;

			FPostProcess& getPostProcessingData() override;

			FRenderWorldStats& getStats() override;
			const FRenderWorldStats& getStats() const override;
		private:
			void prepareIndirectCommands();
			void prepareMeshletCommands();
		protected:
			std::vector<FFrustum> m_vSceneFrustums{};
			std::vector<FCullingInfo> m_vCullingInfo{};

			std::vector<FGeometryRawBatch> m_vRawGeometryBatches{};
			std::vector<FGeometryCPUBatch> m_vGeometryBatches{};
			std::vector<FMaterial> m_vMaterials{};
			bool m_bMaterialsChanged{ true };

			//std::vector<FMaterial> m_vMaterials{};
			std::vector<glm::mat4> m_vSkins{};

			FPostProcess m_postprocess{};
			FRenderWorldStats m_stats{};

			std::vector<std::unique_ptr<IGPUVector>> m_vBuffers{};
		private:
			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}