#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		class CRenderWorld : public IRenderWorld
		{
		public:
			CRenderWorld(IRenderInterface* render_interface, IRenderContext* render_context);
			~CRenderWorld() = default;

			void create() override;

			size_t registerFrustum(uint32_t type) override;
			void updateFrustum(size_t frustum_id, const glm::mat4& view, const glm::mat4& projection) override;

			void addDrawGeometry(const std::vector<FGeometryRawBatch>& new_geometry) override;

			void render(float dt) override;

			void addLight(const FDirectionalLightCommit& light) override;
			void addLight(const FPointLightCommit& light) override;
			void addLight(const FSpotLightCommit& light) override;

			void addShadow(const FCascadedShadowCommit& commit) override;
			void addShadow(const FOmniShadowCommit& commit) override;
			void addShadow(const FSpotShadowCommit& commit) override;

			void updateNode(uint32_t node_id, const FSceneNode& node) override;
			void updateSkin(const std::array<glm::mat4, 128ull>& matrices) override;

			void addMaterial(const FMaterial& material) override;
			void addMaterials(const std::vector<FMaterial>& materials) override;

			const std::vector<std::unique_ptr<IBufferObject>>& getDrawBuffers() override;

			const std::unordered_map<uint32_t, std::vector<FGeometryBatchRange>>& getAllGeometryRanges() const;
			const std::vector<FGeometryBatchRange>& getGeometryRanges(uint32_t type) const;
			const FGeometryCPUBatch& getGeometryBatch(size_t index) const;

			FRenderWorldStats& getStats() override;
			const FRenderWorldStats& getStats() const override;
		protected:
			std::vector<FFrustum> m_vSceneFrustums{};
			std::vector<FCullingInfo> m_vCullingInfo{};

			std::vector<FGeometryRawBatch> m_vRawGeometryBatches{};
			std::vector<FGeometryCPUBatch> m_vGeometryBatches{};
			mutable std::unordered_map<uint32_t, std::vector<FGeometryBatchRange>> m_mBatchRanges{};

			std::vector<FMaterial> m_vMaterials{};
			std::vector<FSceneNode> m_vNodes{};
			std::vector<glm::mat4> m_vSkins{};

			std::vector<FDirectionalLightCommit> m_vDirectionalLights{};
			std::vector<FPointLightCommit> m_vPointLights{};
			std::vector<FSpotLightCommit> m_vSpotLights{};

			std::vector<FCascadedShadowCommit> m_vCascadedShadows{};
			std::vector<FOmniShadowCommit> m_vOmniShadows{};
			std::vector<FSpotShadowCommit> m_vSpotShadows{};

			FRenderWorldStats m_stats{};
		protected:
			std::vector<std::unique_ptr<IBufferObject>> m_vBuffers{};
		private:
			IRenderInterface* m_pRenderInterface{ nullptr };
			IRenderContext* m_pRenderContext{ nullptr };
		};
	}
}