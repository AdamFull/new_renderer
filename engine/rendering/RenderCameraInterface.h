#pragma once

#include <glm/glm.hpp>

namespace engine
{
	namespace render
	{
		enum class RenderCameraMatrixType
		{
			eView = 0,
			eInvView = 1,
			eProj = 2,
			eInvProj = 3,
			eViewProj = 4,
			ePrevViewProj = 5,
			eInvViewProj = 6,
			eReproj = 7,
		};

		class IRenderCamera
		{
		public:
			virtual ~IRenderCamera() = default;

			const glm::vec3 GetForwardVercor() const;
			const glm::vec3 GetRightVercor() const;
			const glm::vec3 GetUpVercor() const;

			void SetFOV(float new_fov);
			float GetFOV() const;

			void SetClip(float n, float f);
			float GetClipNear() const;
			float GetClipFar() const;

			virtual void Update(const glm::vec3& translation, const glm::quat& rotation) = 0;

			virtual const glm::mat4& GetMatrix(RenderCameraMatrixType type) const = 0;
		protected:
			glm::vec3 forward{};
			glm::vec3 right{};
			glm::vec3 up{};

			float fov{ 45.f };
			float nearp{ 0.1f };
			float farp{ 100.f };

			bool dirty{ true };
		};
	}
}