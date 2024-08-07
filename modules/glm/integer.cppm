module;

#include <glm/integer.hpp>

export module glm.integer;

export namespace glm
{
	using glm::uaddCarry;
	using glm::usubBorrow;
	using glm::umulExtended;
	using glm::imulExtended;
	using glm::bitfieldExtract;
	using glm::bitfieldInsert;
	using glm::bitfieldReverse;
	using glm::bitCount;
	using glm::findLSB;
	using glm::findMSB;
}