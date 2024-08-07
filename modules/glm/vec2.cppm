module;

#include <glm/vec2.hpp>
#include <glm/packing.hpp>

export module glm.vec2;

export namespace glm
{
	using glm::bvec2;

	using glm::i8vec2;
	using glm::u8vec2;

	using glm::i16vec2;
	using glm::u16vec2;

	using glm::ivec2;
	using glm::uvec2;

	using glm::vec2;
	using glm::dvec2;

	using glm::i64vec2;
	using glm::u64vec2;

	// Functions packing
	using glm::packUnorm2x16;
	using glm::packSnorm2x16;
	using glm::unpackUnorm2x16;
	using glm::unpackSnorm2x16;

	using glm::packDouble2x32;
	using glm::unpackDouble2x32;

	using glm::packHalf2x16;
	using glm::unpackHalf2x16;
}
