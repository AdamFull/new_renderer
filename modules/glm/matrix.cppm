module;

#include <glm/matrix.hpp>

export module glm.matrix;

//import glm.vec2;
//import glm.vec3;
//import glm.vec4;
//
//import glm.mat2;
//import glm.mat3;
//import glm.mat4;

export namespace glm
{
	using glm::matrixCompMult;
	using glm::outerProduct;
	using glm::transpose;
	using glm::determinant;
	using glm::inverse;
}