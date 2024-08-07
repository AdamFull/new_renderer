module;

#include <glm/vec4.hpp>
#include <glm/packing.hpp>

export module glm.vec4;

export namespace glm
{
	using glm::bvec4;

	using glm::i8vec4;
	using glm::u8vec4;

	using glm::i16vec4;
	using glm::u16vec4;

	using glm::ivec4;
	using glm::uvec4;

	using glm::vec4;
	using glm::dvec4;

	using glm::i64vec4;
	using glm::u64vec4;

	// Functions packing
	using glm::packUnorm4x8;
	using glm::packSnorm4x8;
	using glm::unpackUnorm4x8;
	using glm::unpackSnorm4x8;
}
