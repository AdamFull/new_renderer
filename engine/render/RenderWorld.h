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

			void Create() override;

			u64 RegisterCamera(ERenderGeometryCullerType type) override;
			void UpdateCamera(u64 camera_id, const glm::mat4& view, const glm::mat4& prev_view, const glm::mat4& projection, const glm::mat4& prev_projection, const glm::vec2& size, float znear = 0.f, float zfar = 1.f) override;
			const FCamera& GetCamera(u64 index) const override;
			const FCamera& GetDefaultCamera() const override;
			const u64 GetCameraID(ERenderGeometryCullerType type) const override;
			const u64 GetDefaultCameraID() const override;

			void AddDrawGeometry(const utl::vector<FGeometryBatch>& new_geometry) override;

			void Begin(f32 dt) override;
			void End() override;

			u64 UpdateLight(const FLightCommit& light) override;

			void AddShadow(const FCascadedShadowCommit& commit) override;
			void AddShadow(const FOmniShadowCommit& commit) override;
			void AddShadow(const FSpotShadowCommit& commit) override;

			void AddMatrices(const utl::vector<glm::mat4>& matrices) override;
			const glm::mat4& GetMatrix(u64 index) const override;

			u64 UpdateNode(u32 node_id, const FSceneNode& node) override;

			void AddMaterial(const FMaterial& material) override;
			void AddMaterials(const utl::vector<FMaterial>& materials) override;
			const FMaterial& GetMaterial(u64 index) const override;

			void AddMesh(const FMesh& mesh) override;
			void AddMeshes(const utl::vector<FMesh>& meshes) override;

			const std::unique_ptr<IGPUVector>& GetDrawBuffer(ERenderBufferIndices buffer_index) const override;
			const utl::vector<FGeometryCPUBatch>& GetGeometryBatches() const override;
			const utl::vector<FDrawInfo>& GetDrawCommandByType(ERenderGeometryCullerType type) const override;

			FPostProcess& GetPostProcessingData() override;

			FRenderWorldStats& GetStats() override;
			const FRenderWorldStats& GetStats() const override;
		private:
			void prepareIndirectCommands();
			void prepareMeshletCommands();
		protected:
			utl::hash_map<ERenderGeometryCullerType, utl::vector<FDrawInfo>> m_perCullerDrawCommands{};
			utl::vector<FFrustum> m_vSceneFrustums{};
			utl::vector<FCullingInfo> m_vCullingInfo{};
			u64 m_defaultCameraID{ 0ull };

			utl::hash_map<u32, u64> m_nodeRemap{};

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