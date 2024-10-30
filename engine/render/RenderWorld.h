#pragma once

#include <engine/interface/render/RenderInterface.h>

namespace engine
{
	namespace render
	{
		struct FFrustum
		{
		public:
			void update(const glm::mat4& view, const glm::mat4& projection)
			{
				clip[0] = view[0][0] * projection[0][0] + view[0][1] * projection[1][0] + view[0][2] * projection[2][0] + view[0][3] * projection[3][0];
				clip[1] = view[0][0] * projection[0][1] + view[0][1] * projection[1][1] + view[0][2] * projection[2][1] + view[0][3] * projection[3][1];
				clip[2] = view[0][0] * projection[0][2] + view[0][1] * projection[1][2] + view[0][2] * projection[2][2] + view[0][3] * projection[3][2];
				clip[3] = view[0][0] * projection[0][3] + view[0][1] * projection[1][3] + view[0][2] * projection[2][3] + view[0][3] * projection[3][3];

				clip[4] = view[1][0] * projection[0][0] + view[1][1] * projection[1][0] + view[1][2] * projection[2][0] + view[1][3] * projection[3][0];
				clip[5] = view[1][0] * projection[0][1] + view[1][1] * projection[1][1] + view[1][2] * projection[2][1] + view[1][3] * projection[3][1];
				clip[6] = view[1][0] * projection[0][2] + view[1][1] * projection[1][2] + view[1][2] * projection[2][2] + view[1][3] * projection[3][2];
				clip[7] = view[1][0] * projection[0][3] + view[1][1] * projection[1][3] + view[1][2] * projection[2][3] + view[1][3] * projection[3][3];

				clip[8] = view[2][0] * projection[0][0] + view[2][1] * projection[1][0] + view[2][2] * projection[2][0] + view[2][3] * projection[3][0];
				clip[9] = view[2][0] * projection[0][1] + view[2][1] * projection[1][1] + view[2][2] * projection[2][1] + view[2][3] * projection[3][1];
				clip[10] = view[2][0] * projection[0][2] + view[2][1] * projection[1][2] + view[2][2] * projection[2][2] + view[2][3] * projection[3][2];
				clip[11] = view[2][0] * projection[0][3] + view[2][1] * projection[1][3] + view[2][2] * projection[2][3] + view[2][3] * projection[3][3];

				clip[12] = view[3][0] * projection[0][0] + view[3][1] * projection[1][0] + view[3][2] * projection[2][0] + view[3][3] * projection[3][0];
				clip[13] = view[3][0] * projection[0][1] + view[3][1] * projection[1][1] + view[3][2] * projection[2][1] + view[3][3] * projection[3][1];
				clip[14] = view[3][0] * projection[0][2] + view[3][1] * projection[1][2] + view[3][2] * projection[2][2] + view[3][3] * projection[3][2];
				clip[15] = view[3][0] * projection[0][3] + view[3][1] * projection[1][3] + view[3][2] * projection[2][3] + view[3][3] * projection[3][3];

				//Left
				frustumSides[1][0] = clip[3] - clip[0];
				frustumSides[1][1] = clip[7] - clip[4];
				frustumSides[1][2] = clip[11] - clip[8];
				frustumSides[1][3] = clip[15] - clip[12];
				normalizeFrustumSide(1);

				//Right
				frustumSides[0][0] = clip[3] + clip[0];
				frustumSides[0][1] = clip[7] + clip[4];
				frustumSides[0][2] = clip[11] + clip[8];
				frustumSides[0][3] = clip[15] + clip[12];
				normalizeFrustumSide(0);

				//Top
				frustumSides[3][0] = clip[3] - clip[1];
				frustumSides[3][1] = clip[7] - clip[5];
				frustumSides[3][2] = clip[11] - clip[9];
				frustumSides[3][3] = clip[15] - clip[13];
				normalizeFrustumSide(3);

				//Bottom
				frustumSides[2][0] = clip[3] + clip[1];
				frustumSides[2][1] = clip[7] + clip[5];
				frustumSides[2][2] = clip[11] + clip[9];
				frustumSides[2][3] = clip[15] + clip[13];
				normalizeFrustumSide(2);

				//Back
				frustumSides[4][0] = clip[3] + clip[2];
				frustumSides[4][1] = clip[7] + clip[6];
				frustumSides[4][2] = clip[11] + clip[10];
				frustumSides[4][3] = clip[15] + clip[14];
				normalizeFrustumSide(4);

				//Front
				frustumSides[5][0] = clip[3] - clip[2];
				frustumSides[5][1] = clip[7] - clip[6];
				frustumSides[5][2] = clip[11] - clip[10];
				frustumSides[5][3] = clip[15] - clip[14];
			}

			/**
			 * @brief Check is point in frustum
			 *
			 * @param pos Point position
			 * @return true when point is in frustum
			 */
			bool checkPoint(const glm::vec3& pos) const
			{
				for (u32 i = 0; i < 6; i++)
				{
					if (frustumSides[i].x * pos.x + frustumSides[i].y * pos.y + frustumSides[i].z * pos.z + frustumSides[i].w <= 0.0f)
						return false;
				}
				return true;
			}

			/**
			 * @brief Check is sphere intersects with frustum
			 *
			 * @param pos Sphere center position
			 * @param radius Sphere radius
			 * @return true when sphere intersects with camera frustum
			 */
			bool checkSphere(const glm::vec3& pos, f32 radius) const
			{
				for (u32 i = 0; i < 6; i++)
				{
					if (frustumSides[i].x * pos.x + frustumSides[i].y * pos.y + frustumSides[i].z * pos.z + frustumSides[i].w <= -radius)
						return false;
				}
				return true;
			}

			/**
			 * @brief Check is bounding box intersects with frustum
			 *
			 * @param start Bounding box start point
			 * @param end Bounding box end boint
			 * @return true when bounding box intersects with camera frustum
			 */
			bool checkBox(const glm::vec3& start, const glm::vec3& end) const
			{
				for (u32 i = 0; i < 6; i++)
				{
					if (frustumSides[i].x * start.x + frustumSides[i].y * start.y + frustumSides[i].z * start.z + frustumSides[i].w <= 0.0f
						&& frustumSides[i].x * end.x + frustumSides[i].y * start.y + frustumSides[i].z * start.z + frustumSides[i].w <= 0.0f
						&& frustumSides[i].x * start.x + frustumSides[i].y * end.y + frustumSides[i].z * start.z + frustumSides[i].w <= 0.0f
						&& frustumSides[i].x * end.x + frustumSides[i].y * end.y + frustumSides[i].z * start.z + frustumSides[i].w <= 0.0f
						&& frustumSides[i].x * start.x + frustumSides[i].y * start.y + frustumSides[i].z * end.z + frustumSides[i].w <= 0.0f
						&& frustumSides[i].x * end.x + frustumSides[i].y * start.y + frustumSides[i].z * end.z + frustumSides[i].w <= 0.0f
						&& frustumSides[i].x * start.x + frustumSides[i].y * end.y + frustumSides[i].z * end.z + frustumSides[i].w <= 0.0f
						&& frustumSides[i].x * end.x + frustumSides[i].y * end.y + frustumSides[i].z * end.z + frustumSides[i].w <= 0.0f)
						return false;
				}
				return true;
			}

			std::array<glm::vec4, 6> frustumSides{};
			std::array<f32, 16> clip;
		private:
			void normalizeFrustumSide(u64 side)
			{
				auto magnitude = std::sqrt(frustumSides[side][0] * frustumSides[side][0] + frustumSides[side][1] * frustumSides[side][1] + frustumSides[side][2] * frustumSides[side][2]);
				frustumSides[side][0] /= magnitude;
				frustumSides[side][1] /= magnitude;
				frustumSides[side][2] /= magnitude;
				frustumSides[side][3] /= magnitude;
			}
		};

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