module;

#include <glm/gtc/color_space.hpp>

export module glm.gtc.color_space;

export namespace glm
{
	using glm::convertLinearToSRGB;
	using glm::convertSRGBToLinear;
}