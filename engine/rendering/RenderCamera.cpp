#include "RenderCamera.h"

#include <glm/gtx/quaternion.hpp>

namespace engine
{
	namespace render
	{
		void Frustum::Update(const glm::mat4& view, const glm::mat4& projection)
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
			frustum_sides[1][0] = clip[3] - clip[0];
			frustum_sides[1][1] = clip[7] - clip[4];
			frustum_sides[1][2] = clip[11] - clip[8];
			frustum_sides[1][3] = clip[15] - clip[12];
			NormalizeFrustumSide(1);

			//Right
			frustum_sides[0][0] = clip[3] + clip[0];
			frustum_sides[0][1] = clip[7] + clip[4];
			frustum_sides[0][2] = clip[11] + clip[8];
			frustum_sides[0][3] = clip[15] + clip[12];
			NormalizeFrustumSide(0);

			//Top
			frustum_sides[3][0] = clip[3] - clip[1];
			frustum_sides[3][1] = clip[7] - clip[5];
			frustum_sides[3][2] = clip[11] - clip[9];
			frustum_sides[3][3] = clip[15] - clip[13];
			NormalizeFrustumSide(3);

			//Bottom
			frustum_sides[2][0] = clip[3] + clip[1];
			frustum_sides[2][1] = clip[7] + clip[5];
			frustum_sides[2][2] = clip[11] + clip[9];
			frustum_sides[2][3] = clip[15] + clip[13];
			NormalizeFrustumSide(2);

			//Back
			frustum_sides[4][0] = clip[3] + clip[2];
			frustum_sides[4][1] = clip[7] + clip[6];
			frustum_sides[4][2] = clip[11] + clip[10];
			frustum_sides[4][3] = clip[15] + clip[14];
			NormalizeFrustumSide(4);

			//Front
			frustum_sides[5][0] = clip[3] - clip[2];
			frustum_sides[5][1] = clip[7] - clip[6];
			frustum_sides[5][2] = clip[11] - clip[10];
			frustum_sides[5][3] = clip[15] - clip[14];
		}

		bool Frustum::CheckPoint(const glm::vec3& pos) const
		{
			for (int32_t i = 0; i < 6; i++)
			{
				if (frustum_sides[i].x * pos.x + frustum_sides[i].y * pos.y + frustum_sides[i].z * pos.z + frustum_sides[i].w <= 0.0f)
					return false;
			}
			return true;
		}

		bool Frustum::CheckSphere(const glm::vec3& pos, float radius) const
		{
			for (int32_t i = 0; i < 6; i++)
			{
				if (frustum_sides[i].x * pos.x + frustum_sides[i].y * pos.y + frustum_sides[i].z * pos.z + frustum_sides[i].w <= -radius)
					return false;
			}
			return true;
		}

		bool Frustum::CheckBox(const glm::vec3& start, const glm::vec3& end) const
		{
			for (int32_t i = 0; i < 6; i++)
			{
				if (frustum_sides[i].x * start.x + frustum_sides[i].y * start.y + frustum_sides[i].z * start.z + frustum_sides[i].w <= 0.0f
					&& frustum_sides[i].x * end.x + frustum_sides[i].y * start.y + frustum_sides[i].z * start.z + frustum_sides[i].w <= 0.0f
					&& frustum_sides[i].x * start.x + frustum_sides[i].y * end.y + frustum_sides[i].z * start.z + frustum_sides[i].w <= 0.0f
					&& frustum_sides[i].x * end.x + frustum_sides[i].y * end.y + frustum_sides[i].z * start.z + frustum_sides[i].w <= 0.0f
					&& frustum_sides[i].x * start.x + frustum_sides[i].y * start.y + frustum_sides[i].z * end.z + frustum_sides[i].w <= 0.0f
					&& frustum_sides[i].x * end.x + frustum_sides[i].y * start.y + frustum_sides[i].z * end.z + frustum_sides[i].w <= 0.0f
					&& frustum_sides[i].x * start.x + frustum_sides[i].y * end.y + frustum_sides[i].z * end.z + frustum_sides[i].w <= 0.0f
					&& frustum_sides[i].x * end.x + frustum_sides[i].y * end.y + frustum_sides[i].z * end.z + frustum_sides[i].w <= 0.0f)
					return false;
			}
			return true;
		}

		void Frustum::NormalizeFrustumSide(int32_t side)
		{
			auto magnitude = std::sqrt(frustum_sides[side][0] * frustum_sides[side][0] + frustum_sides[side][1] * frustum_sides[side][1] + frustum_sides[side][2] * frustum_sides[side][2]);
			frustum_sides[side][0] /= magnitude;
			frustum_sides[side][1] /= magnitude;
			frustum_sides[side][2] /= magnitude;
			frustum_sides[side][3] /= magnitude;
		}



		const glm::vec3 IRenderCamera::GetForwardVercor() const
		{
			return forward;
		}

		const glm::vec3 IRenderCamera::GetRightVercor() const
		{
			return right;
		}

		const glm::vec3 IRenderCamera::GetUpVercor() const
		{
			return up;
		}

		void IRenderCamera::SetFOV(float new_fov)
		{
			if (fov != new_fov)
			{
				fov = new_fov;
				dirty = true;
			}
		}

		float IRenderCamera::GetFOV() const
		{
			return fov;
		}

		void IRenderCamera::SetClip(float n, float f)
		{
			if (nearp != n || farp != f)
			{
				nearp = n; 
				farp = f;
				dirty = true;
			}
		}

		float IRenderCamera::GetClipNear() const
		{
			return nearp;
		}

		float IRenderCamera::GetClipFar() const
		{
			return farp;
		}


		void RenderCamera::Update(const glm::vec3& translation, const glm::quat& rotation)
		{
			float screen_size_x = 1920.f;
			float screen_size_y = 1080.f;

			if (dirty)
			{
				forward = glm::normalize(glm::rotate(rotation, glm::vec3(0.f, 0.f, -1.f)));
				right = glm::normalize(glm::cross(forward, glm::vec3{ 0.f, 1.f, 0.f }));
				up = glm::normalize(glm::cross(right, forward));

				matrices[utl::enum_cast(RenderCameraMatrixType::eView)] = glm::lookAt(translation, translation + forward, up);
				matrices[utl::enum_cast(RenderCameraMatrixType::eInvView)] = glm::inverse(matrices[utl::enum_cast(RenderCameraMatrixType::eView)]);
				matrices[utl::enum_cast(RenderCameraMatrixType::eProj)] = glm::perspective(fov, screen_size_x / screen_size_y, nearp, farp);
				matrices[utl::enum_cast(RenderCameraMatrixType::eInvProj)] = glm::inverse(matrices[utl::enum_cast(RenderCameraMatrixType::eProj)]);
				matrices[utl::enum_cast(RenderCameraMatrixType::ePrevViewProj)] = matrices[utl::enum_cast(RenderCameraMatrixType::eViewProj)];
				matrices[utl::enum_cast(RenderCameraMatrixType::eViewProj)] = matrices[utl::enum_cast(RenderCameraMatrixType::eProj)] * matrices[utl::enum_cast(RenderCameraMatrixType::eView)];
				matrices[utl::enum_cast(RenderCameraMatrixType::eInvViewProj)] = glm::inverse(matrices[utl::enum_cast(RenderCameraMatrixType::eViewProj)]);

				// Update frustum
				frustum.Update(matrices[utl::enum_cast(RenderCameraMatrixType::eView)], matrices[utl::enum_cast(RenderCameraMatrixType::eProj)]);

				auto& camera_reprojection = matrices[utl::enum_cast(RenderCameraMatrixType::eReproj)];

				camera_reprojection = GetMatrix(RenderCameraMatrixType::ePrevViewProj) * GetMatrix(RenderCameraMatrixType::eInvViewProj);

				float rcp_half_dimx = 2.0f / screen_size_x;
				float rcp_half_dimy = 2.0f / screen_size_y;
				float rcp_z_magic = nearp / (farp - nearp);

				bool linear_z = false;

				// Transform from uv to clip space: texcoord * 2 - 1
				static glm::mat4 to_clip_space =
				{
					rcp_half_dimx, 0.0f, 0.0f, 0.0f,
					0.0f, -rcp_half_dimy, 0.0f, 0.0f,
					0.0f, 0.0f, linear_z ? rcp_z_magic : 1.0f, 0.0f,
					-1.0f, 1.0f, linear_z ? -rcp_z_magic : 0.0f, 1.0f
				};

				// Transform from clip to uv space: texcoord * 0.5 + 0.5
				static glm::mat4 to_uv_space =
				{
					1.0f / rcp_half_dimx, 0.0f, 0.0f, 0.0f,
					0.0f, -1.0f / rcp_half_dimy, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					1.0f / rcp_half_dimx, 1.0f / rcp_half_dimy, 0.0f, 1.0f
				};
				camera_reprojection = to_clip_space * camera_reprojection * to_uv_space;

				dirty = false;


				// TODO: Write data to buffers
			}
		}
		
		const glm::mat4& RenderCamera::GetMatrix(RenderCameraMatrixType type) const
		{
			return matrices[utl::enum_cast(type)];
		}

		void RenderCamera::Flush(IRenderer& renderer)
		{

		}
	}
}