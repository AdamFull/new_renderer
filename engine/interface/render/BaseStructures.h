#pragma once

#if defined(PLATFORM_WIN32)
#include <Windows.h>
#endif

#include <string>
#include <vector>
#include <array>

#include <glm/glm.hpp>

namespace engine
{
	namespace render
	{
		enum class ERenderApi
		{
			eVulkan10,
			eVulkan11,
			eVulkan12,
			eVulkan13,
			eOpenGL,
			eDirectX12
		};

		struct FRenderInterfaceCreateInfo
		{
			ERenderApi render_api{};

#if defined( PLATFORM_WIN32 )
			HINSTANCE hinstance;
			HWND hwnd;
#endif

			int32_t initialWidth{};
			int32_t initialHeight{};
		};

		struct FRenderContextCreateInfo
		{
			std::string appName{ "dummy_app" };
			ERenderApi apiVersion{};

#if defined( VK_USE_PLATFORM_WIN32_KHR )
			HINSTANCE                       hinstance;
			HWND                            hwnd;
#endif
			std::vector<char> pipelineCache{};

			std::vector<std::string> requiredDeviceExtensions{};
			std::vector<std::string> optionalDeviceExtensions{};

			uint32_t initialWidth{ 0u };
			uint32_t initialHeight{ 0u };

			bool validationState{ false };
			uint32_t validationFeatures{ 0u };
			std::vector<std::string> validationLayers{};
		};

		template<class _Ty>
		struct FPoint2D
		{
			FPoint2D() = default;
			FPoint2D(_Ty val) : x(val), y(val) {}
			FPoint2D(_Ty _x, _Ty _y) : x(_x), y(_y) {}

			_Ty x{ 0 }, y{ 0 };
		};

		template<class _Ty>
		struct FPoint3D
		{
			FPoint3D() = default;
			FPoint3D(_Ty val) : x(val), y(val), z(val) {}
			FPoint3D(_Ty _x, _Ty _y, _Ty _z) : x(_x), y(_y), z(_z) {}

			_Ty x{ 0 }, y{ 0 }, z{ 0 };
		};

		template<class _Ty>
		struct FRect2D
		{
			FRect2D() = default;
			FRect2D(const FPoint2D<_Ty>& _offset, const FPoint2D<_Ty>& _size) : offset(_offset), size(_size) {}
			FRect2D(_Ty offset_x, _Ty offset_y, _Ty size_x, _Ty size_y) : offset(offset_x, offset_y), size(size_x, size_y) {}

			FPoint2D<_Ty> offset{};
			FPoint2D<_Ty> size{};
		};

		struct FGeometryRanges
		{
			size_t beginIndex{ 0ull }, indexCount{ 0ull };
			size_t beginVertex{ 0ull }, vertexCount{ 0ull };
		};

		struct FGeometryDrawData
		{
			size_t vboID{ ~0ull };
			size_t materialID{ ~0ull };
			uint32_t nodeID{ ~0u };

			FGeometryRanges range{};
		};

		struct FGeometryBatch
		{
			size_t vboID{ ~0ull };
			size_t materialID{ ~0ull };
			uint32_t nodeID{ ~0u };

			std::vector<FGeometryRanges> ranges{};
		};

		struct FGeometryDrawInfo
		{
			size_t lastIndex{ 0ull };
			size_t lastVertex{ 0ull };
			std::vector<FGeometryRanges> ranges{};
		};

		struct FIndirectGeometryDrawInfo
		{
			size_t bufferId{ ~0ull };
			size_t offset{ 0ull };
			size_t count{ 0ull };
		};

		struct FGeometryDrawCall
		{
			size_t vboId{ ~0ull };
			size_t materialId{ ~0ull };
			uint32_t node{ ~0u };

			FIndirectGeometryDrawInfo command{};
		};

		struct FDispatchGroup
		{
			uint32_t x, y, z{};
		};

		struct FDispatchInfo
		{
			std::vector<FDispatchGroup> groups{};
		};

		struct FAPIObjectDestroyInfo
		{
			size_t id{ ~0ull };
			uint32_t mask{ 0u };
		};

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
				for (uint32_t i = 0; i < 6; i++)
				{
					if (frustumSides[i][0] * pos.x + frustumSides[i][1] * pos.y + frustumSides[i][2] * pos.z + frustumSides[i][3] <= 0.0f)
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
			bool checkSphere(const glm::vec3& pos, float radius) const
			{
				for (uint32_t i = 0; i < 6; i++)
				{
					if (frustumSides[i][0] * pos.x + frustumSides[i][1] * pos.y + frustumSides[i][2] * pos.z + frustumSides[i][3] <= -radius)
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
				for (uint32_t i = 0; i < 6; i++)
				{
					if (frustumSides[i][0] * start.x + frustumSides[i][1] * start.y + frustumSides[i][2] * start.z + frustumSides[i][3] <= 0.0f
						&& frustumSides[i][0] * end.x + frustumSides[i][1] * start.y + frustumSides[i][2] * start.z + frustumSides[i][3] <= 0.0f
						&& frustumSides[i][0] * start.x + frustumSides[i][1] * end.y + frustumSides[i][2] * start.z + frustumSides[i][3] <= 0.0f
						&& frustumSides[i][0] * end.x + frustumSides[i][1] * end.y + frustumSides[i][2] * start.z + frustumSides[i][3] <= 0.0f
						&& frustumSides[i][0] * start.x + frustumSides[i][1] * start.y + frustumSides[i][2] * end.z + frustumSides[i][3] <= 0.0f
						&& frustumSides[i][0] * end.x + frustumSides[i][1] * start.y + frustumSides[i][2] * end.z + frustumSides[i][3] <= 0.0f
						&& frustumSides[i][0] * start.x + frustumSides[i][1] * end.y + frustumSides[i][2] * end.z + frustumSides[i][3] <= 0.0f
						&& frustumSides[i][0] * end.x + frustumSides[i][1] * end.y + frustumSides[i][2] * end.z + frustumSides[i][3] <= 0.0f)
						return false;
				}
				return true;
			}

			std::array<std::array<float, 4>, 6> frustumSides{};
			std::array<float, 16> clip;
		private:
			void normalizeFrustumSide(size_t side)
			{
				auto magnitude = std::sqrt(frustumSides[side][0] * frustumSides[side][0] + frustumSides[side][1] * frustumSides[side][1] + frustumSides[side][2] * frustumSides[side][2]);
				frustumSides[side][0] /= magnitude;
				frustumSides[side][1] /= magnitude;
				frustumSides[side][2] /= magnitude;
				frustumSides[side][3] /= magnitude;
			}
		};
	}
}