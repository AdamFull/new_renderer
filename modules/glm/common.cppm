module;

#include <glm/common.hpp>

export module glm.common;

export namespace glm
{
	using glm::abs;
	using glm::sign;
	using glm::floor;
	using glm::trunc;
	using glm::round;
	using glm::roundEven;
	using glm::ceil;
	using glm::fract;
	using glm::mod;
	using glm::modf;
	using glm::min;
	using glm::max;
	using glm::clamp;
	using glm::mix;
	using glm::step;
	using glm::smoothstep;
	using glm::isnan;
	using glm::isinf;
	using glm::floatBitsToInt;
	using glm::floatBitsToUint;
	using glm::intBitsToFloat;
	using glm::uintBitsToFloat;
	using glm::fma;
	using glm::frexp;
	using glm::ldexp;
}