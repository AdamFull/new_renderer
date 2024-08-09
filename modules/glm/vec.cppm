module;

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_aligned.hpp>
#include <glm/geometric.hpp>
#include <glm/vector_relational.hpp>
#include <glm/gtc/type_ptr.hpp>

export module glm.vec;

export namespace glm
{
	// Type default

	/// 2 components vector of boolean.
	using glm::bvec2;

	/// 8 bit signed integer vector of 2 components type.
	using glm::i8vec2;

	/// 16 bit signed integer vector of 2 components type.
	using glm::i16vec2;

	/// 32 bit signed integer vector of 2 components type.
	using glm::i32vec2;

	/// 64 bit signed integer vector of 2 components type.
	using glm::i64vec2;

	/// 8 bit unsigned integer vector of 2 components type.
	using glm::u8vec2;

	/// 16 bit unsigned integer vector of 2 components type.
	using glm::u16vec2;

	/// 32 bit unsigned integer vector of 2 components type.
	using glm::u32vec2;

	/// 64 bit unsigned integer vector of 2 components type.
	using glm::u64vec2;

	/// 2 components vector of signed integer numbers.
	using glm::ivec2;

	/// 2 components vector of unsigned integer numbers.
	using glm::uvec2;

	/// 2 components vector of single-precision floating-point numbers.
	using glm::vec2;

	/// 2 components vector of double-precision floating-point numbers.
	using glm::dvec2;
	

	/// 3 components vector of boolean.
	using glm::bvec3;

	/// 8 bit signed integer vector of 3 components type.
	using glm::i8vec3;

	/// 16 bit signed integer vector of 3 components type.
	using glm::i16vec3;

	/// 32 bit signed integer vector of 3 components type.
	using glm::i32vec3;

	/// 64 bit signed integer vector of 3 components type.
	using glm::i64vec3;

	/// 8 bit unsigned integer vector of 3 components type.
	using glm::u8vec3;

	/// 16 bit unsigned integer vector of 3 components type.
	using glm::u16vec3;

	/// 32 bit unsigned integer vector of 3 components type.
	using glm::u32vec3;

	/// 64 bit unsigned integer vector of 3 components type.
	using glm::u64vec3;

	/// 3 components vector of single-precision floating-point numbers.
	using glm::vec3;

	/// 3 components vector of double-precision floating-point numbers.
	using glm::dvec3;

	
	/// 4 components vector of boolean.
	using glm::bvec4;

	/// 8 bit signed integer vector of 4 components type.
	///
	/// @see ext_vector_int4_sized
	using glm::i8vec4;

	/// 16 bit signed integer vector of 4 components type.
	///
	/// @see ext_vector_int4_sized
	using glm::i16vec4;

	/// 32 bit signed integer vector of 4 components type.
	///
	/// @see ext_vector_int4_sized
	using glm::i32vec4;

	/// 64 bit signed integer vector of 4 components type.
	///
	/// @see ext_vector_int4_sized
	using glm::i64vec4;

	/// 8 bit unsigned integer vector of 4 components type.
	///
	/// @see ext_vector_uint4_sized
	using glm::u8vec4;

	/// 16 bit unsigned integer vector of 4 components type.
	///
	/// @see ext_vector_uint4_sized
	using glm::u16vec4;

	/// 32 bit unsigned integer vector of 4 components type.
	///
	/// @see ext_vector_uint4_sized
	using glm::u32vec4;

	/// 64 bit unsigned integer vector of 4 components type.
	///
	/// @see ext_vector_uint4_sized
	using glm::u64vec4;

	/// 4 components vector of single-precision floating-point numbers.
	using glm::vec4;

	/// 4 components vector of double-precision floating-point numbers.
	using glm::dvec4;

	// Type aligned

	// -- *vec1 --

	/// 1 component vector aligned in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_vec1;

	/// 1 component vector aligned in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_vec1;

	/// 1 component vector aligned in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_vec1;

	/// 1 component vector aligned in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_dvec1;

	/// 1 component vector aligned in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_dvec1;

	/// 1 component vector aligned in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_dvec1;

	/// 1 component vector aligned in memory of signed integer numbers.
	using glm::aligned_highp_ivec1;

	/// 1 component vector aligned in memory of signed integer numbers.
	using glm::aligned_mediump_ivec1;

	/// 1 component vector aligned in memory of signed integer numbers.
	using glm::aligned_lowp_ivec1;

	/// 1 component vector aligned in memory of unsigned integer numbers.
	using glm::aligned_highp_uvec1;

	/// 1 component vector aligned in memory of unsigned integer numbers.
	using glm::aligned_mediump_uvec1;

	/// 1 component vector aligned in memory of unsigned integer numbers.
	using glm::aligned_lowp_uvec1;

	/// 1 component vector aligned in memory of bool values.
	using glm::aligned_highp_bvec1;

	/// 1 component vector aligned in memory of bool values.
	using glm::aligned_mediump_bvec1;

	/// 1 component vector aligned in memory of bool values.
	using glm::aligned_lowp_bvec1;

	/// 1 component vector tightly packed in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_vec1;

	/// 1 component vector tightly packed in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_vec1;

	/// 1 component vector tightly packed in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_vec1;

	/// 1 component vector tightly packed in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_dvec1;

	/// 1 component vector tightly packed in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_dvec1;

	/// 1 component vector tightly packed in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_dvec1;

	/// 1 component vector tightly packed in memory of signed integer numbers.
	using glm::packed_highp_ivec1;

	/// 1 component vector tightly packed in memory of signed integer numbers.
	using glm::packed_mediump_ivec1;

	/// 1 component vector tightly packed in memory of signed integer numbers.
	using glm::packed_lowp_ivec1;

	/// 1 component vector tightly packed in memory of unsigned integer numbers.
	using glm::packed_highp_uvec1;

	/// 1 component vector tightly packed in memory of unsigned integer numbers.
	using glm::packed_mediump_uvec1;

	/// 1 component vector tightly packed in memory of unsigned integer numbers.
	using glm::packed_lowp_uvec1;

	/// 1 component vector tightly packed in memory of bool values.
	using glm::packed_highp_bvec1;

	/// 1 component vector tightly packed in memory of bool values.
	using glm::packed_mediump_bvec1;

	/// 1 component vector tightly packed in memory of bool values.
	using glm::packed_lowp_bvec1;

	// -- *vec2 --

	/// 2 components vector aligned in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_vec2;

	/// 2 components vector aligned in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_vec2;

	/// 2 components vector aligned in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_vec2;

	/// 2 components vector aligned in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_dvec2;

	/// 2 components vector aligned in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_dvec2;

	/// 2 components vector aligned in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_dvec2;

	/// 2 components vector aligned in memory of signed integer numbers.
	using glm::aligned_highp_ivec2;

	/// 2 components vector aligned in memory of signed integer numbers.
	using glm::aligned_mediump_ivec2;

	/// 2 components vector aligned in memory of signed integer numbers.
	using glm::aligned_lowp_ivec2;

	/// 2 components vector aligned in memory of unsigned integer numbers.
	using glm::aligned_highp_uvec2;

	/// 2 components vector aligned in memory of unsigned integer numbers.
	using glm::aligned_mediump_uvec2;

	/// 2 components vector aligned in memory of unsigned integer numbers.
	using glm::aligned_lowp_uvec2;

	/// 2 components vector aligned in memory of bool values.
	using glm::aligned_highp_bvec2;

	/// 2 components vector aligned in memory of bool values.
	using glm::aligned_mediump_bvec2;

	/// 2 components vector aligned in memory of bool values.
	using glm::aligned_lowp_bvec2;

	/// 2 components vector tightly packed in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_vec2;

	/// 2 components vector tightly packed in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_vec2;

	/// 2 components vector tightly packed in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_vec2;

	/// 2 components vector tightly packed in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_dvec2;

	/// 2 components vector tightly packed in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_dvec2;

	/// 2 components vector tightly packed in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_dvec2;

	/// 2 components vector tightly packed in memory of signed integer numbers.
	using glm::packed_highp_ivec2;

	/// 2 components vector tightly packed in memory of signed integer numbers.
	using glm::packed_mediump_ivec2;

	/// 2 components vector tightly packed in memory of signed integer numbers.
	using glm::packed_lowp_ivec2;

	/// 2 components vector tightly packed in memory of unsigned integer numbers.
	using glm::packed_highp_uvec2;

	/// 2 components vector tightly packed in memory of unsigned integer numbers.
	using glm::packed_mediump_uvec2;

	/// 2 components vector tightly packed in memory of unsigned integer numbers.
	using glm::packed_lowp_uvec2;

	/// 2 components vector tightly packed in memory of bool values.
	using glm::packed_highp_bvec2;

	/// 2 components vector tightly packed in memory of bool values.
	using glm::packed_mediump_bvec2;

	/// 2 components vector tightly packed in memory of bool values.
	using glm::packed_lowp_bvec2;
	
	// -- *vec3 --

	/// 3 components vector aligned in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_vec3;

	/// 3 components vector aligned in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_vec3;

	/// 3 components vector aligned in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_vec3;

	/// 3 components vector aligned in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_dvec3;

	/// 3 components vector aligned in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_dvec3;

	/// 3 components vector aligned in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_dvec3;

	/// 3 components vector aligned in memory of signed integer numbers.
	using glm::aligned_highp_ivec3;

	/// 3 components vector aligned in memory of signed integer numbers.
	using glm::aligned_mediump_ivec3;

	/// 3 components vector aligned in memory of signed integer numbers.
	using glm::aligned_lowp_ivec3;

	/// 3 components vector aligned in memory of unsigned integer numbers.
	using glm::aligned_highp_uvec3;

	/// 3 components vector aligned in memory of unsigned integer numbers.
	using glm::aligned_mediump_uvec3;

	/// 3 components vector aligned in memory of unsigned integer numbers.
	using glm::aligned_lowp_uvec3;

	/// 3 components vector aligned in memory of bool values.
	using glm::aligned_highp_bvec3;

	/// 3 components vector aligned in memory of bool values.
	using glm::aligned_mediump_bvec3;

	/// 3 components vector aligned in memory of bool values.
	using glm::aligned_lowp_bvec3;

	/// 3 components vector tightly packed in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_vec3;

	/// 3 components vector tightly packed in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_vec3;

	/// 3 components vector tightly packed in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_vec3;

	/// 3 components vector tightly packed in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_dvec3;

	/// 3 components vector tightly packed in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_dvec3;

	/// 3 components vector tightly packed in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_dvec3;

	/// 3 components vector tightly packed in memory of signed integer numbers.
	using glm::packed_highp_ivec3;

	/// 3 components vector tightly packed in memory of signed integer numbers.
	using glm::packed_mediump_ivec3;

	/// 3 components vector tightly packed in memory of signed integer numbers.
	using glm::packed_lowp_ivec3;

	/// 3 components vector tightly packed in memory of unsigned integer numbers.
	using glm::packed_highp_uvec3;

	/// 3 components vector tightly packed in memory of unsigned integer numbers.
	using glm::packed_mediump_uvec3;

	/// 3 components vector tightly packed in memory of unsigned integer numbers.
	using glm::packed_lowp_uvec3;

	/// 3 components vector tightly packed in memory of bool values.
	using glm::packed_highp_bvec3;

	/// 3 components vector tightly packed in memory of bool values.
	using glm::packed_mediump_bvec3;

	/// 3 components vector tightly packed in memory of bool values.
	using glm::packed_lowp_bvec3;

	// -- *vec4 --

	/// 4 components vector aligned in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_vec4;

	/// 4 components vector aligned in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_vec4;

	/// 4 components vector aligned in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_vec4;

	/// 4 components vector aligned in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_dvec4;

	/// 4 components vector aligned in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_dvec4;

	/// 4 components vector aligned in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_dvec4;

	/// 4 components vector aligned in memory of signed integer numbers.
	using glm::aligned_highp_ivec4;

	/// 4 components vector aligned in memory of signed integer numbers.
	using glm::aligned_mediump_ivec4;

	/// 4 components vector aligned in memory of signed integer numbers.
	using glm::aligned_lowp_ivec4;

	/// 4 components vector aligned in memory of unsigned integer numbers.
	using glm::aligned_highp_uvec4;

	/// 4 components vector aligned in memory of unsigned integer numbers.
	using glm::aligned_mediump_uvec4;

	/// 4 components vector aligned in memory of unsigned integer numbers.
	using glm::aligned_lowp_uvec4;

	/// 4 components vector aligned in memory of bool values.
	using glm::aligned_highp_bvec4;

	/// 4 components vector aligned in memory of bool values.
	using glm::aligned_mediump_bvec4;

	/// 4 components vector aligned in memory of bool values.
	using glm::aligned_lowp_bvec4;

	/// 4 components vector tightly packed in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_vec4;

	/// 4 components vector tightly packed in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_vec4;

	/// 4 components vector tightly packed in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_vec4;

	/// 4 components vector tightly packed in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_dvec4;

	/// 4 components vector tightly packed in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_dvec4;

	/// 4 components vector tightly packed in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_dvec4;

	/// 4 components vector tightly packed in memory of signed integer numbers.
	using glm::packed_highp_ivec4;

	/// 4 components vector tightly packed in memory of signed integer numbers.
	using glm::packed_mediump_ivec4;

	/// 4 components vector tightly packed in memory of signed integer numbers.
	using glm::packed_lowp_ivec4;

	/// 4 components vector tightly packed in memory of unsigned integer numbers.
	using glm::packed_highp_uvec4;

	/// 4 components vector tightly packed in memory of unsigned integer numbers.
	using glm::packed_mediump_uvec4;

	/// 4 components vector tightly packed in memory of unsigned integer numbers.
	using glm::packed_lowp_uvec4;

	/// 4 components vector tightly packed in memory of bool values.
	using glm::packed_highp_bvec4;

	/// 4 components vector tightly packed in memory of bool values.
	using glm::packed_mediump_bvec4;

	/// 4 components vector tightly packed in memory of bool values.
	using glm::packed_lowp_bvec4;


	// -- default --
	/// 1 component vector aligned in memory of single-precision floating-point numbers.
	using glm::aligned_vec1;

	/// 2 components vector aligned in memory of single-precision floating-point numbers.
	using glm::aligned_vec2;

	/// 3 components vector aligned in memory of single-precision floating-point numbers.
	using glm::aligned_vec3;

	/// 4 components vector aligned in memory of single-precision floating-point numbers.
	using glm::aligned_vec4;

	/// 1 component vector tightly packed in memory of single-precision floating-point numbers.
	using glm::packed_vec1;

	/// 2 components vector tightly packed in memory of single-precision floating-point numbers.
	using glm::packed_vec2;

	/// 3 components vector tightly packed in memory of single-precision floating-point numbers.
	using glm::packed_vec3;

	/// 4 components vector tightly packed in memory of single-precision floating-point numbers.
	using glm::packed_vec4;

	/// 1 component vector aligned in memory of double-precision floating-point numbers.
	using glm::aligned_dvec1;

	/// 2 components vector aligned in memory of double-precision floating-point numbers.
	using glm::aligned_dvec2;

	/// 3 components vector aligned in memory of double-precision floating-point numbers.
	using glm::aligned_dvec3;

	/// 4 components vector aligned in memory of double-precision floating-point numbers.
	using glm::aligned_dvec4;

	/// 1 component vector tightly packed in memory of double-precision floating-point numbers.
	using glm::packed_dvec1;

	/// 2 components vector tightly packed in memory of double-precision floating-point numbers.
	using glm::packed_dvec2;

	/// 3 components vector tightly packed in memory of double-precision floating-point numbers.
	using glm::packed_dvec3;

	/// 4 components vector tightly packed in memory of double-precision floating-point numbers.
	using glm::packed_dvec4;

	/// 1 component vector aligned in memory of signed integer numbers.
	using glm::aligned_ivec1;

	/// 2 components vector aligned in memory of signed integer numbers.
	using glm::aligned_ivec2;

	/// 3 components vector aligned in memory of signed integer numbers.
	using glm::aligned_ivec3;

	/// 4 components vector aligned in memory of signed integer numbers.
	using glm::aligned_ivec4;

	/// 1 component vector tightly packed in memory of signed integer numbers.
	using glm::packed_ivec1;

	/// 2 components vector tightly packed in memory of signed integer numbers.
	using glm::packed_ivec2;

	/// 3 components vector tightly packed in memory of signed integer numbers.
	using glm::packed_ivec3;

	/// 4 components vector tightly packed in memory of signed integer numbers.
	using glm::packed_ivec4;

	// -- Unsigned integer definition --

	/// 1 component vector aligned in memory of unsigned integer numbers.
	using glm::aligned_uvec1;

	/// 2 components vector aligned in memory of unsigned integer numbers.
	using glm::aligned_uvec2;

	/// 3 components vector aligned in memory of unsigned integer numbers.
	using glm::aligned_uvec3;

	/// 4 components vector aligned in memory of unsigned integer numbers.
	using glm::aligned_uvec4;

	/// 1 component vector tightly packed in memory of unsigned integer numbers.
	using glm::packed_uvec1;

	/// 2 components vector tightly packed in memory of unsigned integer numbers.
	using glm::packed_uvec2;

	/// 3 components vector tightly packed in memory of unsigned integer numbers.
	using glm::packed_uvec3;

	/// 4 components vector tightly packed in memory of unsigned integer numbers.
	using glm::packed_uvec4;

	/// 1 component vector aligned in memory of bool values.
	using glm::aligned_bvec1;

	/// 2 components vector aligned in memory of bool values.
	using glm::aligned_bvec2;

	/// 3 components vector aligned in memory of bool values.
	using glm::aligned_bvec3;

	/// 4 components vector aligned in memory of bool values.
	using glm::aligned_bvec4;

	/// 1 components vector tightly packed in memory of bool values.
	using glm::packed_bvec1;

	/// 2 components vector tightly packed in memory of bool values.
	using glm::packed_bvec2;

	/// 3 components vector tightly packed in memory of bool values.
	using glm::packed_bvec3;

	/// 4 components vector tightly packed in memory of bool values.
	using glm::packed_bvec4;

	// Relational

	/// Returns the component-wise comparison result of x < y.
	using glm::lessThan;

	// Returns the component-wise comparison of result x <= y.
	using glm::lessThanEqual;

	// Returns the component-wise comparison of result x > y.
	using glm::greaterThan;

	/// Returns the component-wise comparison of result x >= y.
	using glm::greaterThanEqual;

	/// Returns the component-wise comparison of result x == y.
	using glm::equal;

	/// Returns the component-wise comparison of result x != y.
	using glm::notEqual;

	/// Returns true if any component of x is true.
	using glm::any;

	/// Returns true if all components of x are true.
	using glm::all;

	/// Returns the component-wise logical complement of x.
	using glm::not_;

	// Geometric

	/// Returns the length of x, i.e., sqrt(x * x).
	using glm::length;

	/// Returns the distance between p0 and p1, i.e., length(p0 - p1).
	using glm::distance;

	/// Returns the dot product of x and y, i.e., result = x * y.
	using glm::dot;

	/// Returns the cross product of x and y.
	using glm::cross;

	/// Returns a vector in the same direction as x but with length of 1.
	/// According to issue 10 GLSL 1.10 specification, if length(x) == 0 then result is undefined and generate an error.
	using glm::normalize;

	/// If dot(Nref, I) < 0.0, return N, otherwise, return -N.
	using glm::faceforward;

	/// For the incident vector I and surface orientation N,
	/// returns the reflection direction : result = I - 2.0 * dot(N, I) * N.
	using glm::reflect;

	/// For the incident vector I and surface normal N,
	/// and the ratio of indices of refraction eta,
	/// return the refraction vector.
	using glm::refract;

	// Type ptr

	/// Return the constant address to the data of the input parameter.
	using glm::value_ptr;

	/// Build a vector from a pointer.
	using glm::make_vec1;

	/// Build a vector from a pointer.
	using glm::make_vec1;

	/// Build a vector from a pointer.
	using glm::make_vec1;

	/// Build a vector from a pointer.
	using glm::make_vec1;

	/// Build a vector from a pointer.
	using glm::make_vec2;

	/// Build a vector from a pointer.
	using glm::make_vec2;

	/// Build a vector from a pointer.
	using glm::make_vec2;

	/// Build a vector from a pointer.
	using glm::make_vec2;

	/// Build a vector from a pointer.
	using glm::make_vec3;

	/// Build a vector from a pointer.
	using glm::make_vec3;

	/// Build a vector from a pointer.
	using glm::make_vec3;

	/// Build a vector from a pointer.
	using glm::make_vec3;

	/// Build a vector from a pointer.
	using glm::make_vec4;

	/// Build a vector from a pointer.
	using glm::make_vec4;

	/// Build a vector from a pointer.
	using glm::make_vec4;

	/// Build a vector from a pointer.
	using glm::make_vec4;

	/// Build a vector from a pointer.
	using glm::make_vec2;

	/// Build a vector from a pointer.
	using glm::make_vec3;

	/// Build a vector from a pointer.
	using glm::make_vec4;
}
