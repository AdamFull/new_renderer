module;

#include <glm/mat2x2.hpp>
#include <glm/mat2x3.hpp>
#include <glm/mat2x4.hpp>

#include <glm/mat3x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat3x4.hpp>

#include <glm/mat4x2.hpp>
#include <glm/mat4x3.hpp>
#include <glm/mat4x4.hpp>

#include <glm/gtc/matrix_integer.hpp>
#include <glm/gtc/type_aligned.hpp>

#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/type_ptr.hpp>

export module glm.mat2;

export namespace glm
{
	// -- *mat2 --

	using glm::mat2;
	using glm::dmat2;

	using glm::mat2x3;
	using glm::dmat2x3;

	using glm::mat2x4;
	using glm::dmat2x4;

	using glm::highp_imat2;

	using glm::highp_imat2x2;
	using glm::highp_imat2x3;
	using glm::highp_imat2x4;

	using glm::mediump_imat2;

	using glm::mediump_imat2x2;
	using glm::mediump_imat2x3;
	using glm::mediump_imat2x4;

	using glm::lowp_imat2;

	using glm::lowp_imat2x2;
	using glm::lowp_imat2x3;
	using glm::lowp_imat2x4;

	using glm::highp_umat2;

	using glm::highp_umat2x2;
	using glm::highp_umat2x3;
	using glm::highp_umat2x4;

	using glm::mediump_umat2;

	using glm::mediump_umat2x2;
	using glm::mediump_umat2x3;
	using glm::mediump_umat2x4;

	using glm::lowp_umat2;

	using glm::lowp_umat2x2;
	using glm::lowp_umat2x3;
	using glm::lowp_umat2x4;

	using glm::imat2;

	using glm::imat2x2;
	using glm::imat2x3;
	using glm::imat2x4;

	using glm::umat2;

	using glm::umat2x2;
	using glm::umat2x3;
	using glm::umat2x4;



	using glm::mat3x2;
	using glm::dmat3x2;

	using glm::mat3;
	using glm::dmat3;

	using glm::mat3x4;
	using glm::dmat3x4;

	using glm::highp_imat3;

	using glm::highp_imat3x2;
	using glm::highp_imat3x3;
	using glm::highp_imat3x4;

	using glm::mediump_imat3;

	using glm::mediump_imat3x2;
	using glm::mediump_imat3x3;
	using glm::mediump_imat3x4;

	using glm::lowp_imat3;

	using glm::lowp_imat3x2;
	using glm::lowp_imat3x3;
	using glm::lowp_imat3x4;

	using glm::highp_umat3;

	using glm::highp_umat3x2;
	using glm::highp_umat3x3;
	using glm::highp_umat3x4;

	using glm::mediump_umat3;

	using glm::mediump_umat3x2;
	using glm::mediump_umat3x3;
	using glm::mediump_umat3x4;

	using glm::lowp_umat3;

	using glm::lowp_umat3x2;
	using glm::lowp_umat3x3;
	using glm::lowp_umat3x4;

	using glm::imat3;

	using glm::imat3x2;
	using glm::imat3x3;
	using glm::imat3x4;

	using glm::umat3;

	using glm::umat3x2;
	using glm::umat3x3;
	using glm::umat3x4;


	using glm::mat4x2;
	using glm::dmat4x2;

	using glm::mat4x3;
	using glm::dmat4x3;

	using glm::mat4;
	using glm::dmat4;

	using glm::highp_imat4;

	using glm::highp_imat4x2;
	using glm::highp_imat4x3;
	using glm::highp_imat4x4;

	using glm::mediump_imat4;

	using glm::mediump_imat4x2;
	using glm::mediump_imat4x3;
	using glm::mediump_imat4x4;

	using glm::lowp_imat4;

	using glm::lowp_imat4x2;
	using glm::lowp_imat4x3;
	using glm::lowp_imat4x4;

	using glm::highp_umat4;

	using glm::highp_umat4x2;
	using glm::highp_umat4x3;
	using glm::highp_umat4x4;

	using glm::mediump_umat4;

	using glm::mediump_umat4x2;
	using glm::mediump_umat4x3;
	using glm::mediump_umat4x4;

	using glm::lowp_umat4;

	using glm::lowp_umat4x2;
	using glm::lowp_umat4x3;
	using glm::lowp_umat4x4;

	using glm::imat4;

	using glm::imat4x2;
	using glm::imat4x3;
	using glm::imat4x4;

	using glm::umat4;

	using glm::umat4x2;
	using glm::umat4x3;
	using glm::umat4x4;

	// Type aligned

	// -- *mat2 --

	/// 2 by 2 matrix aligned in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_mat2;

	/// 2 by 2 matrix aligned in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_mat2;

	/// 2 by 2 matrix aligned in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_mat2;

	/// 2 by 2 matrix aligned in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_dmat2;

	/// 2 by 2 matrix aligned in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_dmat2;

	/// 2 by 2 matrix aligned in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_dmat2;

	/// 2 by 2 matrix tightly packed in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_mat2;

	/// 2 by 2 matrix tightly packed in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_mat2;

	/// 2 by 2 matrix tightly packed in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_mat2;

	/// 2 by 2 matrix tightly packed in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_dmat2;

	/// 2 by 2 matrix tightly packed in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_dmat2;

	/// 2 by 2 matrix tightly packed in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_dmat2;

	// -- *mat3 --

	/// 3 by 3 matrix aligned in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_mat3;

	/// 3 by 3 matrix aligned in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_mat3;

	/// 3 by 3 matrix aligned in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_mat3;

	/// 3 by 3 matrix aligned in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_dmat3;

	/// 3 by 3 matrix aligned in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_dmat3;

	/// 3 by 3 matrix aligned in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_dmat3;

	/// 3 by 3 matrix tightly packed in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_mat3;

	/// 3 by 3 matrix tightly packed in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_mat3;

	/// 3 by 3 matrix tightly packed in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_mat3;

	/// 3 by 3 matrix tightly packed in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_dmat3;

	/// 3 by 3 matrix tightly packed in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_dmat3;

	/// 3 by 3 matrix tightly packed in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_dmat3;

	// -- *mat4 --

	/// 4 by 4 matrix aligned in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_mat4;

	/// 4 by 4 matrix aligned in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_mat4;

	/// 4 by 4 matrix aligned in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_mat4;

	/// 4 by 4 matrix aligned in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_dmat4;

	/// 4 by 4 matrix aligned in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_dmat4;

	/// 4 by 4 matrix aligned in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_dmat4;

	/// 4 by 4 matrix tightly packed in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_mat4;

	/// 4 by 4 matrix tightly packed in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_mat4;

	/// 4 by 4 matrix tightly packed in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_mat4;

	/// 4 by 4 matrix tightly packed in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_dmat4;

	/// 4 by 4 matrix tightly packed in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_dmat4;

	/// 4 by 4 matrix tightly packed in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_dmat4;

	// -- *mat2x2 --

	/// 2 by 2 matrix aligned in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_mat2x2;

	/// 2 by 2 matrix aligned in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_mat2x2;

	/// 2 by 2 matrix aligned in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_mat2x2;

	/// 2 by 2 matrix aligned in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_dmat2x2;

	/// 2 by 2 matrix aligned in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_dmat2x2;

	/// 2 by 2 matrix aligned in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_dmat2x2;

	/// 2 by 2 matrix tightly packed in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_mat2x2;

	/// 2 by 2 matrix tightly packed in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_mat2x2;

	/// 2 by 2 matrix tightly packed in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_mat2x2;

	/// 2 by 2 matrix tightly packed in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_dmat2x2;

	/// 2 by 2 matrix tightly packed in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_dmat2x2;

	/// 2 by 2 matrix tightly packed in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_dmat2x2;

	// -- *mat2x3 --

	/// 2 by 3 matrix aligned in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_mat2x3;

	/// 2 by 3 matrix aligned in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_mat2x3;

	/// 2 by 3 matrix aligned in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_mat2x3;

	/// 2 by 3 matrix aligned in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_dmat2x3;

	/// 2 by 3 matrix aligned in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_dmat2x3;

	/// 2 by 3 matrix aligned in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_dmat2x3;

	/// 2 by 3 matrix tightly packed in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_mat2x3;

	/// 2 by 3 matrix tightly packed in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_mat2x3;

	/// 2 by 3 matrix tightly packed in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_mat2x3;

	/// 2 by 3 matrix tightly packed in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_dmat2x3;

	/// 2 by 3 matrix tightly packed in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_dmat2x3;

	/// 2 by 3 matrix tightly packed in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_dmat2x3;

	// -- *mat2x4 --

	/// 2 by 4 matrix aligned in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_mat2x4;

	/// 2 by 4 matrix aligned in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_mat2x4;

	/// 2 by 4 matrix aligned in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_mat2x4;

	/// 2 by 4 matrix aligned in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_dmat2x4;

	/// 2 by 4 matrix aligned in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_dmat2x4;

	/// 2 by 4 matrix aligned in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_dmat2x4;

	/// 2 by 4 matrix tightly packed in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_mat2x4;

	/// 2 by 4 matrix tightly packed in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_mat2x4;

	/// 2 by 4 matrix tightly packed in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_mat2x4;

	/// 2 by 4 matrix tightly packed in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_dmat2x4;

	/// 2 by 4 matrix tightly packed in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_dmat2x4;

	/// 2 by 4 matrix tightly packed in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_dmat2x4;

	// -- *mat3x2 --

	/// 3 by 2 matrix aligned in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_mat3x2;

	/// 3 by 2 matrix aligned in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_mat3x2;

	/// 3 by 2 matrix aligned in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_mat3x2;

	/// 3 by 2 matrix aligned in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_dmat3x2;

	/// 3 by 2 matrix aligned in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_dmat3x2;

	/// 3 by 2 matrix aligned in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_dmat3x2;

	/// 3 by 2 matrix tightly packed in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_mat3x2;

	/// 3 by 2 matrix tightly packed in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_mat3x2;

	/// 3 by 2 matrix tightly packed in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_mat3x2;

	/// 3 by 2 matrix tightly packed in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_dmat3x2;

	/// 3 by 2 matrix tightly packed in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_dmat3x2;

	/// 3 by 2 matrix tightly packed in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_dmat3x2;

	// -- *mat3x3 --

	/// 3 by 3 matrix aligned in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_mat3x3;

	/// 3 by 3 matrix aligned in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_mat3x3;

	/// 3 by 3 matrix aligned in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_mat3x3;

	/// 3 by 3 matrix aligned in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_dmat3x3;

	/// 3 by 3 matrix aligned in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_dmat3x3;

	/// 3 by 3 matrix aligned in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_dmat3x3;

	/// 3 by 3 matrix tightly packed in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_mat3x3;

	/// 3 by 3 matrix tightly packed in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_mat3x3;

	/// 3 by 3 matrix tightly packed in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_mat3x3;

	/// 3 by 3 matrix tightly packed in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_dmat3x3;

	/// 3 by 3 matrix tightly packed in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_dmat3x3;

	/// 3 by 3 matrix tightly packed in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_dmat3x3;

	// -- *mat3x4 --

	/// 3 by 4 matrix aligned in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_mat3x4;

	/// 3 by 4 matrix aligned in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_mat3x4;

	/// 3 by 4 matrix aligned in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_mat3x4;

	/// 3 by 4 matrix aligned in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_dmat3x4;

	/// 3 by 4 matrix aligned in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_dmat3x4;

	/// 3 by 4 matrix aligned in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_dmat3x4;

	/// 3 by 4 matrix tightly packed in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_mat3x4;

	/// 3 by 4 matrix tightly packed in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_mat3x4;

	/// 3 by 4 matrix tightly packed in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_mat3x4;

	/// 3 by 4 matrix tightly packed in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_dmat3x4;

	/// 3 by 4 matrix tightly packed in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_dmat3x4;

	/// 3 by 4 matrix tightly packed in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_dmat3x4;

	// -- *mat4x2 --

	/// 4 by 2 matrix aligned in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_mat4x2;

	/// 4 by 2 matrix aligned in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_mat4x2;

	/// 4 by 2 matrix aligned in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_mat4x2;

	/// 4 by 2 matrix aligned in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_dmat4x2;

	/// 4 by 2 matrix aligned in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_dmat4x2;

	/// 4 by 2 matrix aligned in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_dmat4x2;

	/// 4 by 2 matrix tightly packed in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_mat4x2;

	/// 4 by 2 matrix tightly packed in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_mat4x2;

	/// 4 by 2 matrix tightly packed in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_mat4x2;

	/// 4 by 2 matrix tightly packed in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_dmat4x2;

	/// 4 by 2 matrix tightly packed in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_dmat4x2;

	/// 4 by 2 matrix tightly packed in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_dmat4x2;

	// -- *mat4x3 --

	/// 4 by 3 matrix aligned in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_mat4x3;

	/// 4 by 3 matrix aligned in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_mat4x3;

	/// 4 by 3 matrix aligned in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_mat4x3;

	/// 4 by 3 matrix aligned in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_dmat4x3;

	/// 4 by 3 matrix aligned in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_dmat4x3;

	/// 4 by 3 matrix aligned in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_dmat4x3;

	/// 4 by 3 matrix tightly packed in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_mat4x3;

	/// 4 by 3 matrix tightly packed in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_mat4x3;

	/// 4 by 3 matrix tightly packed in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_mat4x3;

	/// 4 by 3 matrix tightly packed in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_dmat4x3;

	/// 4 by 3 matrix tightly packed in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_dmat4x3;

	/// 4 by 3 matrix tightly packed in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_dmat4x3;

	// -- *mat4x4 --

	/// 4 by 4 matrix aligned in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_mat4x4;

	/// 4 by 4 matrix aligned in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_mat4x4;

	/// 4 by 4 matrix aligned in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_mat4x4;

	/// 4 by 4 matrix aligned in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::aligned_highp_dmat4x4;

	/// 4 by 4 matrix aligned in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::aligned_mediump_dmat4x4;

	/// 4 by 4 matrix aligned in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::aligned_lowp_dmat4x4;

	/// 4 by 4 matrix tightly packed in memory of single-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_mat4x4;

	/// 4 by 4 matrix tightly packed in memory of single-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_mat4x4;

	/// 4 by 4 matrix tightly packed in memory of single-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_mat4x4;

	/// 4 by 4 matrix tightly packed in memory of double-precision floating-point numbers using high precision arithmetic in term of ULPs.
	using glm::packed_highp_dmat4x4;

	/// 4 by 4 matrix tightly packed in memory of double-precision floating-point numbers using medium precision arithmetic in term of ULPs.
	using glm::packed_mediump_dmat4x4;

	/// 4 by 4 matrix tightly packed in memory of double-precision floating-point numbers using low precision arithmetic in term of ULPs.
	using glm::packed_lowp_dmat4x4;

	// -- default --

	/// 2 by 2 matrix tightly aligned in memory of single-precision floating-point numbers.
	using glm::aligned_mat2;

	/// 3 by 3 matrix tightly aligned in memory of single-precision floating-point numbers.
	using glm::aligned_mat3;

	/// 4 by 4 matrix tightly aligned in memory of single-precision floating-point numbers.
	using glm::aligned_mat4;

	/// 2 by 2 matrix tightly packed in memory of single-precision floating-point numbers.
	using glm::packed_mat2;

	/// 3 by 3 matrix tightly packed in memory of single-precision floating-point numbers.
	using glm::packed_mat3;

	/// 4 by 4 matrix tightly packed in memory of single-precision floating-point numbers.
	using glm::packed_mat4;

	/// 2 by 2 matrix tightly aligned in memory of single-precision floating-point numbers.
	using glm::aligned_mat2x2;

	/// 2 by 3 matrix tightly aligned in memory of single-precision floating-point numbers.
	using glm::aligned_mat2x3;

	/// 2 by 4 matrix tightly aligned in memory of single-precision floating-point numbers.
	using glm::aligned_mat2x4;

	/// 3 by 2 matrix tightly aligned in memory of single-precision floating-point numbers.
	using glm::aligned_mat3x2;

	/// 3 by 3 matrix tightly aligned in memory of single-precision floating-point numbers.
	using glm::aligned_mat3x3;

	/// 3 by 4 matrix tightly aligned in memory of single-precision floating-point numbers.
	using glm::aligned_mat3x4;

	/// 4 by 2 matrix tightly aligned in memory of single-precision floating-point numbers.
	using glm::aligned_mat4x2;

	/// 4 by 3 matrix tightly aligned in memory of single-precision floating-point numbers.
	using glm::aligned_mat4x3;

	/// 4 by 4 matrix tightly aligned in memory of single-precision floating-point numbers.
	using glm::aligned_mat4x4;

	/// 2 by 2 matrix tightly packed in memory of single-precision floating-point numbers.
	using glm::packed_mat2x2;

	/// 2 by 3 matrix tightly packed in memory of single-precision floating-point numbers.
	using glm::packed_mat2x3;

	/// 2 by 4 matrix tightly packed in memory of single-precision floating-point numbers.
	using glm::packed_mat2x4;

	/// 3 by 2 matrix tightly packed in memory of single-precision floating-point numbers.
	using glm::packed_mat3x2;

	/// 3 by 3 matrix tightly packed in memory of single-precision floating-point numbers.
	using glm::packed_mat3x3;

	/// 3 by 4 matrix tightly packed in memory of single-precision floating-point numbers.
	using glm::packed_mat3x4;

	/// 4 by 2 matrix tightly packed in memory of single-precision floating-point numbers.
	using glm::packed_mat4x2;

	/// 4 by 3 matrix tightly packed in memory of single-precision floating-point numbers.
	using glm::packed_mat4x3;

	/// 4 by 4 matrix tightly packed in memory of single-precision floating-point numbers.
	using glm::packed_mat4x4;

	/// 2 by 2 matrix tightly aligned in memory of double-precision floating-point numbers.
	using glm::aligned_dmat2;

	/// 3 by 3 matrix tightly aligned in memory of double-precision floating-point numbers.
	using glm::aligned_dmat3;

	/// 4 by 4 matrix tightly aligned in memory of double-precision floating-point numbers.
	using glm::aligned_dmat4;

	/// 2 by 2 matrix tightly packed in memory of double-precision floating-point numbers.
	using glm::packed_dmat2;

	/// 3 by 3 matrix tightly packed in memory of double-precision floating-point numbers.
	using glm::packed_dmat3;

	/// 4 by 4 matrix tightly packed in memory of double-precision floating-point numbers.
	using glm::packed_dmat4;

	/// 2 by 2 matrix tightly aligned in memory of double-precision floating-point numbers.
	using glm::aligned_dmat2x2;

	/// 2 by 3 matrix tightly aligned in memory of double-precision floating-point numbers.
	using glm::aligned_dmat2x3;

	/// 2 by 4 matrix tightly aligned in memory of double-precision floating-point numbers.
	using glm::aligned_dmat2x4;

	/// 3 by 2 matrix tightly aligned in memory of double-precision floating-point numbers.
	using glm::aligned_dmat3x2;

	/// 3 by 3 matrix tightly aligned in memory of double-precision floating-point numbers.
	using glm::aligned_dmat3x3;

	/// 3 by 4 matrix tightly aligned in memory of double-precision floating-point numbers.
	using glm::aligned_dmat3x4;

	/// 4 by 2 matrix tightly aligned in memory of double-precision floating-point numbers.
	using glm::aligned_dmat4x2;

	/// 4 by 3 matrix tightly aligned in memory of double-precision floating-point numbers.
	using glm::aligned_dmat4x3;

	/// 4 by 4 matrix tightly aligned in memory of double-precision floating-point numbers.
	using glm::aligned_dmat4x4;

	/// 2 by 2 matrix tightly packed in memory of double-precision floating-point numbers.
	using glm::packed_dmat2x2;

	/// 2 by 3 matrix tightly packed in memory of double-precision floating-point numbers.
	using glm::packed_dmat2x3;

	/// 2 by 4 matrix tightly packed in memory of double-precision floating-point numbers.
	using glm::packed_dmat2x4;

	/// 3 by 2 matrix tightly packed in memory of double-precision floating-point numbers.
	using glm::packed_dmat3x2;

	/// 3 by 3 matrix tightly packed in memory of double-precision floating-point numbers.
	using glm::packed_dmat3x3;

	/// 3 by 4 matrix tightly packed in memory of double-precision floating-point numbers.
	using glm::packed_dmat3x4;

	/// 4 by 2 matrix tightly packed in memory of double-precision floating-point numbers.
	using glm::packed_dmat4x2;

	/// 4 by 3 matrix tightly packed in memory of double-precision floating-point numbers.
	using glm::packed_dmat4x3;

	/// 4 by 4 matrix tightly packed in memory of double-precision floating-point numbers.
	using glm::packed_dmat4x4;


	// Access

	/// Get a specific row of a matrix.
	using glm::row;

	/// Get a specific column of a matrix.
	using glm::column;

	// Inverse

	/// Fast matrix inverse for affine matrix.
	using glm::affineInverse;

	/// Compute the inverse transpose of a matrix.
	using glm::inverseTranspose;

	// Transform

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



	using glm::identity;


	using glm::translate;



	using glm::rotate;


	using glm::rotate_slow;



	using glm::scale;


	using glm::scale_slow;



	using glm::lookAtRH;


	using glm::lookAtLH;


	using glm::lookAt;


	// Type ptr

	/// Build a matrix from a pointer.
	using glm::make_mat2x2;

	/// Build a matrix from a pointer.
	using glm::make_mat2x3;

	/// Build a matrix from a pointer.
	using glm::make_mat2x4;

	/// Build a matrix from a pointer.
	using glm::make_mat3x2;

	/// Build a matrix from a pointer.
	using glm::make_mat3x3;

	/// Build a matrix from a pointer.
	using glm::make_mat3x4;

	/// Build a matrix from a pointer.
	using glm::make_mat4x2;

	/// Build a matrix from a pointer.
	using glm::make_mat4x3;

	/// Build a matrix from a pointer.
	using glm::make_mat4x4;

	/// Build a matrix from a pointer.
	using glm::make_mat2;

	/// Build a matrix from a pointer.
	using glm::make_mat3;

	/// Build a matrix from a pointer.
	using glm::make_mat4;
}
