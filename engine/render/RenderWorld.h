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

			void addDrawGeometry(const stl::vector<FGeometryRawBatch>& new_geometry) override;

			void render(f32 dt) override;

			void addLight(const FDirectionalLightCommit& light) override;
			void addLight(const FPointLightCommit& light) override;
			void addLight(const FSpotLightCommit& light) override;

			void addShadow(const FCascadedShadowCommit& commit) override;
			void addShadow(const FOmniShadowCommit& commit) override;
			void addShadow(const FSpotShadowCommit& commit) override;

			void updateNode(u32 node_id, const FSceneNode& node) override;
			void updateSkin(const stl::array<glm::mat4, 128ull>& matrices) override;

			void addMaterial(const FMaterial& material) override;
			void addMaterials(const stl::vector<FMaterial>& materials) override;

			const stl::unique_ptr<IBufferObject>& getDrawBuffer(ERenderBufferIndices buffer_index) override;
			//const stl::vector<stl::unique_ptr<IBufferObject>>& getDrawBuffers() override;

			const stl::unordered_map<ERenderGeometryCullerType, stl::vector<FGeometryBatchRange>>& getAllGeometryRanges() const override;
			const stl::vector<FGeometryBatchRange>& getGeometryRanges(ERenderGeometryCullerType type) const override;
			const FGeometryCPUBatch& getGeometryBatch(u64 index) const override;

			FRenderWorldStats& getStats() override;
			const FRenderWorldStats& getStats() const override;
		protected:
			stl::vector<FFrustum> m_vSceneFrustums{};
			stl::vector<FCullingInfo> m_vCullingInfo{};

			stl::vector<FGeometryRawBatch> m_vRawGeometryBatches{};
			stl::vector<FGeometryCPUBatch> m_vGeometryBatches{};
			mutable stl::unordered_map<ERenderGeometryCullerType, stl::vector<FGeometryBatchRange>> m_mBatchRanges{};

			stl::vector<FMaterial> m_vMaterials{};
			stl::vector<FSceneNode> m_vNodes{};
			stl::vector<glm::mat4> m_vSkins{};

			stl::vector<FDirectionalLightCommit> m_vDirectionalLights{};
			stl::vector<FPointLightCommit> m_vPointLights{};
			stl::vector<FSpotLightCommit> m_vSpotLights{};

			stl::vector<FCascadedShadowCommit> m_vCascadedShadows{};
			stl::vector<FOmniShadowCommit> m_vOmniShadows{};
			stl::vector<FSpotShadowCommit> m_vSpotShadows{};

			FRenderWorldStats m_stats{};
		protected:
			stl::vector<stl::unique_ptr<IBufferObject>> m_vBuffers{};
		private:
			IRenderInterface* m_pRenderInterface{ nullptr };
		};
	}
}