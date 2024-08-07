module;

#include <glm/gtc/matrix_transform.hpp>

export module glm.gtc.matrix_transform;

export namespace glm
{
	using glm::projectZO;
	using glm::projectNO;
	using glm::project;

	using glm::unProjectZO;
	using glm::unProjectNO;
	using glm::unProject;

	using glm::pickMatrix;

	using glm::orthoLH_ZO;
	using glm::orthoLH_NO;
	using glm::orthoRH_ZO;
	using glm::orthoRH_NO;
	using glm::orthoZO;
	using glm::orthoNO;
	using glm::orthoLH;
	using glm::orthoRH;
	using glm::ortho;

	using glm::frustumLH_ZO;
	using glm::frustumLH_NO;
	using glm::frustumRH_ZO;
	using glm::frustumRH_NO;
	using glm::frustumZO;
	using glm::frustumNO;
	using glm::frustumLH;
	using glm::frustumRH;
	using glm::frustum;

	using glm::perspectiveRH_ZO;
	using glm::perspectiveRH_NO;
	using glm::perspectiveLH_ZO;
	using glm::perspectiveLH_NO;
	using glm::perspectiveZO;
	using glm::perspectiveNO;
	using glm::perspectiveRH;
	using glm::perspectiveLH;
	using glm::perspective;

	using glm::perspectiveFovRH_ZO;
	using glm::perspectiveFovRH_NO;
	using glm::perspectiveFovLH_ZO;
	using glm::perspectiveFovLH_NO;
	using glm::perspectiveFovZO;
	using glm::perspectiveFovNO;
	using glm::perspectiveFovRH;
	using glm::perspectiveFovLH;
	using glm::perspectiveFov;

	using glm::infinitePerspectiveLH;
	using glm::infinitePerspectiveRH;
	using glm::infinitePerspective;

	using glm::tweakedInfinitePerspective;
	using glm::tweakedInfinitePerspective;
}