#pragma once

#include <embed/shaders/includes/interop/base.h>
#include "RenderCameraInterface.h"
#include <array>

namespace engine
{
	namespace render
	{
		class Frustum
		{
		public:
			void Update(const glm::mat4& view, const glm::mat4& projection);

			/**
			 * @brief Check is point in frustum
			 *
			 * @param pos Point position
			 * @return true when point is in frustum
			 */
			bool CheckPoint(const glm::vec3& pos) const;

			/**
			 * @brief Check is sphere intersects with frustum
			 *
			 * @param pos Sphere center position
			 * @param radius Sphere radius
			 * @return true when sphere intersects with camera frustum
			 */
			bool CheckSphere(const glm::vec3& pos, float radius) const;

			/**
			 * @brief Check is bounding box intersects with frustum
			 *
			 * @param start Bounding box start point
			 * @param end Bounding box end boint
			 * @return true when bounding box intersects with camera frustum
			 */
			bool CheckBox(const glm::vec3& start, const glm::vec3& end) const;

			std::array<glm::vec4, 6> frustum_sides{};
			std::array<float, 16> clip;
		private:
			void NormalizeFrustumSide(int32_t side);
		};

		class RenderCamera final : public IRenderCamera
		{
		public:
			RenderCamera() = default;
			~RenderCamera() = default;

			void Update(const glm::vec3& translation, const glm::quat& rotation) override;
			const glm::mat4& GetMatrix(RenderCameraMatrixType type) const override;
			void Flush(IRenderer& renderer) override;
		private:
			Frustum frustum{};

			std::array<glm::mat4, matrix_offset_count> matrices{};
		};
	}
}