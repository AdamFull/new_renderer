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

			u64 registerCamera(ERenderGeometryCullerType type) override;
			void updateCamera(u64 camera_id, const glm::mat4& view, const glm::mat4& prev_view, const glm::mat4& projection, const glm::mat4& prev_projection, const glm::vec2& size, float znear = 0.f, float zfar = 1.f) override;
			const FCamera& getCamera(u64 index) const override;
			const FCamera& getDefaultCamera() const override;
			const u64 getCameraID(ERenderGeometryCullerType type) const override;
			const u64 getDefaultCameraID() const override;

			void addDrawGeometry(const utl::vector<FGeometryBatch>& new_geometry) override;

			void begin(f32 dt) override;
			void end() override;

			void addLight(const FDirectionalLightCommit& light) override;
			void addLight(const FPointLightCommit& light) override;
			void addLight(const FSpotLightCommit& light) override;

			void addShadow(const FCascadedShadowCommit& commit) override;
			void addShadow(const FOmniShadowCommit& commit) override;
			void addShadow(const FSpotShadowCommit& commit) override;

			void addMatrices(const utl::vector<glm::mat4>& matrices) override;
			const glm::mat4& getMatrix(u64 index) const override;

			void updateNode(u32 node_id, const FSceneNode& node) override;

			void addMaterial(const FMaterial& material) override;
			void addMaterials(const utl::vector<FMaterial>& materials) override;
			const FMaterial& getMaterial(u64 index) const override;

			void addMesh(const FMesh& mesh) override;
			void addMeshes(const utl::vector<FMesh>& meshes) override;

			const std::unique_ptr<IGPUVector>& getDrawBuffer(ERenderBufferIndices buffer_index) const override;
			const utl::vector<FGeometryCPUBatch>& getGeometryBatches() const override;
			const utl::vector<FDrawInfo>& getDrawCommandByType(ERenderGeometryCullerType type) const override;

			FPostProcess& getPostProcessingData() override;

			FRenderWorldStats& getStats() override;
			const FRenderWorldStats& getStats() const override;
		private:
			void prepareIndirectCommands();
			void prepareMeshletCommands();
		protected:
			utl::hash_map<ERenderGeometryCullerType, utl::vector<FDrawInfo>> m_perCullerDrawCommands{};
			utl::vector<FFrustum> m_vSceneFrustums{};
			utl::vector<FCullingInfo> m_vCullingInfo{};
			u64 m_defaultCameraID{ 0ull };

			utl::vector<FGeometryBatch> m_vRawGeometryBatches{};
			utl::vector<FGeometryCPUBatch> m_vGeometryBatches{};

			utl::vector<glm::mat4> m_vSkins{};

			FPostProcess m_postprocess{};
			FRenderWorldStats m_stats{};

			utl::vector<std::unique_ptr<IGPUVector>> m_vBuffers{};
		private:
			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}