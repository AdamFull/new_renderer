module;

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

export module glm.quat;

export namespace glm
{
	/// Quaternion of single-precision floating-point numbers.
	using glm::quat;

	/// Quaternion of double-precision floating-point numbers.
	using glm::dquat;

	// Functions

	/// Returns euler angles, pitch as x, yaw as y, roll as z.
	using glm::eulerAngles;

	/// Returns roll value of euler angles expressed in radians.
	using glm::roll;

	/// Returns pitch value of euler angles expressed in radians.
	using glm::pitch;

	/// Returns yaw value of euler angles expressed in radians.
	using glm::yaw;

	/// Converts a quaternion to a 3 * 3 matrix.
	using glm::mat3_cast;

	/// Converts a quaternion to a 4 * 4 matrix.
	using glm::mat4_cast;

	/// Converts a pure rotation 3 * 3 matrix to a quaternion.
	using glm::quat_cast;

	/// Converts a pure rotation 4 * 4 matrix to a quaternion.
	using glm::quat_cast;

	/// Returns the component-wise comparison result of x < y.
	using glm::lessThan;

	/// Returns the component-wise comparison of result x <= y.
	using glm::lessThanEqual;

	/// Returns the component-wise comparison of result x > y.
	using glm::greaterThan;

	/// Returns the component-wise comparison of result x >= y.
	using glm::greaterThanEqual;

	/// Build a look at quaternion based on the default handedness.
	using glm::quatLookAt;

	/// Build a right-handed look at quaternion.
	using glm::quatLookAtRH;

	/// Build a left-handed look at quaternion.
	using glm::quatLookAtLH;

	// Type ptr

	/// Build a quaternion from a pointer.
	using glm::make_quat;
}