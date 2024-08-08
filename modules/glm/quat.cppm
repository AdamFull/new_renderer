module;

#include <glm/gtc/quaternion.hpp>

export module glm.quat;

export namespace glm
{
	using glm::quat;
	using glm::dquat;

	using glm::eulerAngles;
	using glm::roll;
	using glm::pitch;
	using glm::yaw;
	using glm::mat3_cast;
	using glm::mat4_cast;
	using glm::quat_cast;
	using glm::quat_cast;
	using glm::lessThan;
	using glm::lessThanEqual;
	using glm::greaterThan;
	using glm::greaterThanEqual;
	using glm::quatLookAt;
	using glm::quatLookAtRH;
	using glm::quatLookAtLH;
}