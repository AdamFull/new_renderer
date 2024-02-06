#include "RenderConfigBindings.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <sol/sol.hpp>

#include <standart_library.h>

namespace engine
{
	namespace render
	{
        // Helper functions
        template<class _Ty, class _Kty>
        auto make_vec_add()
        {
            return sol::overload(
                [](const _Ty& v1, const _Ty& v2) -> _Ty { return v1 + v2; },
                [](const _Ty& v1, _Kty f) -> _Ty { return v1 + f; },
                [](_Kty f, const _Ty& v1) -> _Ty { return f + v1; }
            );
        }

        template<class _Ty, class _Kty>
        auto make_vec_sub()
        {
            return sol::overload(
                [](const _Ty& v1, const _Ty& v2) -> _Ty { return v1 - v2; },
                [](const _Ty& v1, _Kty f) -> _Ty { return v1 - f; },
                [](_Kty f, const _Ty& v1) -> _Ty { return f - v1; }
            );
        }

        template<class _Ty, class _Kty>
        auto make_vec_mult()
        {
            return sol::overload(
                [](const _Ty& v1, const _Ty& v2) -> _Ty { return v1 * v2; },
                [](const _Ty& v1, _Kty f) -> _Ty { return v1 * f; },
                [](_Kty f, const _Ty& v1) -> _Ty { return f * v1; }
            );
        }

        template<class _Ty, class _Kty>
        auto make_vec_div()
        {
            return sol::overload(
                [](const _Ty& v1, const _Ty& v2) -> _Ty { return v1 / v2; },
                [](const _Ty& v1, _Kty f) -> _Ty { return v1 / f; },
                [](_Kty f, const _Ty& v1) -> _Ty { return f / v1; }
            );
        }

        template<class _Ty, class _Kty>
        auto make_mat_div()
        {
            return sol::overload(
                [](const _Ty& v1, _Kty f) -> _Ty { return v1 / f; },
                [](_Kty f, const _Ty& v1) -> _Ty { return f / v1; }
            );
        }

        template<class _NumType>
        void register_vec1(const char* name, sol::state& lua)
        {
            using namespace glm;
            using _TargetType = vec<1, _NumType, defaultp>;

            lua.new_usertype<_TargetType>(
                name,
                sol::constructors<
                _TargetType(), _TargetType(_NumType),
                _TargetType(const _TargetType&)
                >(),
                sol::meta_function::addition, make_vec_add<_TargetType, _NumType>(),
                sol::meta_function::subtraction, make_vec_sub<_TargetType, _NumType>(),
                sol::meta_function::multiplication, make_vec_mult<_TargetType, _NumType>(),
                sol::meta_function::division, make_vec_div<_TargetType, _NumType>(),
                "x", &_TargetType::x
            );
        }

        template<class _NumType>
        void register_vec2(const char* name, sol::state& lua)
        {
            using namespace glm;
            using _TargetType = vec<2, _NumType, defaultp>;
            using _Vec1Type = vec<1, _NumType, defaultp>;

            lua.new_usertype<_TargetType>(
                name,
                sol::constructors<
                _TargetType(), _TargetType(_NumType), _TargetType(_NumType, _NumType),
                _TargetType(const _TargetType&), _TargetType(const _Vec1Type&), _TargetType(const _Vec1Type&, const _Vec1Type&),
                _TargetType(const _Vec1Type&, _NumType), _TargetType(_NumType, const _Vec1Type&)
                >(),
                sol::meta_function::addition, make_vec_add<_TargetType, _NumType>(),
                sol::meta_function::subtraction, make_vec_sub<_TargetType, _NumType>(),
                sol::meta_function::multiplication, make_vec_mult<_TargetType, _NumType>(),
                sol::meta_function::division, make_vec_div<_TargetType, _NumType>(),
                "x", &_TargetType::x,
                "y", &_TargetType::y
            );
        }

        template<class _NumType>
        void register_vec3(const char* name, sol::state& lua)
        {
            using namespace glm;
            using _TargetType = vec<3, _NumType, defaultp>;
            using _Vec2Type = vec<2, _NumType, defaultp>;
            using _Vec1Type = vec<1, _NumType, defaultp>;

            lua.new_usertype<_TargetType>(
                name,
                sol::constructors<
                _TargetType(), _TargetType(_NumType), _TargetType(_NumType, _NumType, _NumType),
                _TargetType(const _TargetType&), _TargetType(const _Vec1Type&), _TargetType(const _Vec1Type&, const _Vec1Type&, const _Vec1Type&),
                _TargetType(const _Vec2Type&, _NumType), _TargetType(_NumType, const _Vec2Type&)
                >(),
                sol::meta_function::addition, make_vec_add<_TargetType, _NumType>(),
                sol::meta_function::subtraction, make_vec_sub<_TargetType, _NumType>(),
                sol::meta_function::multiplication, make_vec_mult<_TargetType, _NumType>(),
                sol::meta_function::division, make_vec_div<_TargetType, _NumType>(),
                "x", &_TargetType::x,
                "y", &_TargetType::y,
                "z", &_TargetType::z
            );
        }

        template<class _NumType>
        void register_vec4(const char* name, sol::state& lua)
        {
            using namespace glm;
            using _TargetType = vec<4, _NumType, defaultp>;
            using _Vec3Type = vec<3, _NumType, defaultp>;
            using _Vec2Type = vec<2, _NumType, defaultp>;
            using _Vec1Type = vec<1, _NumType, defaultp>;

            lua.new_usertype<_TargetType>(
                name,
                sol::constructors<
                _TargetType(), _TargetType(_NumType), _TargetType(_NumType, _NumType, _NumType, _NumType),
                _TargetType(const _TargetType&), _TargetType(const _Vec1Type&), _TargetType(const _Vec1Type&, const _Vec1Type&, const _Vec1Type&, const _Vec1Type&),
                _TargetType(const _Vec2Type&, _NumType, _NumType), _TargetType(_NumType, const _Vec2Type&, _NumType), _TargetType(_NumType, _NumType, const _Vec2Type&), _TargetType(const _Vec2Type&, const _Vec2Type&),
                _TargetType(const _Vec3Type&, _NumType), _TargetType(_NumType, const _Vec3Type&)
                >(),
                sol::meta_function::addition, make_vec_add<_TargetType, _NumType>(),
                sol::meta_function::subtraction, make_vec_sub<_TargetType, _NumType>(),
                sol::meta_function::multiplication, make_vec_mult<_TargetType, _NumType>(),
                sol::meta_function::division, make_vec_div<_TargetType, _NumType>(),
                "x", &_TargetType::x,
                "y", &_TargetType::y,
                "z", &_TargetType::z,
                "w", &_TargetType::w
            );
        }

        template<class _NumType>
        void register_mat2(const char* name, sol::state& lua)
        {
            using namespace glm;

            using _Vec2Type = vec<2, _NumType, defaultp>;
            using _TargetType = mat<2, 2, _NumType, defaultp>;

            lua.new_usertype<_TargetType>(
                name,
                sol::constructors<
                _TargetType(), _TargetType(_NumType), _TargetType(_NumType, _NumType, _NumType, _NumType),
                _TargetType(const _TargetType&),
                _TargetType(const _Vec2Type&, const _Vec2Type&)
                >(),
                sol::meta_function::addition, make_vec_add<_TargetType, _NumType>(),
                sol::meta_function::subtraction, make_vec_sub<_TargetType, _NumType>(),
                sol::meta_function::multiplication, make_vec_mult<_TargetType, _NumType>(),
                sol::meta_function::division, make_mat_div<_TargetType, _NumType>()
            );
        }

        template<class _NumType>
        void register_mat3(const char* name, sol::state& lua)
        {
            using namespace glm;

            using _Vec1Type = vec<1, _NumType, defaultp>;
            using _Vec3Type = vec<3, _NumType, defaultp>;
            using _TargetType = mat<3, 3, _NumType, defaultp>;

            lua.new_usertype<_TargetType>(
                name,
                sol::constructors<
                _TargetType(), _TargetType(_NumType), _TargetType(_NumType, _NumType, _NumType, _NumType, _NumType, _NumType, _NumType, _NumType, _NumType),
                _TargetType(const _TargetType&),
                _TargetType(const _Vec1Type&, const _Vec1Type&, const _Vec1Type&, const _Vec1Type&, const _Vec1Type&, const _Vec1Type&, const _Vec1Type&, const _Vec1Type&, const _Vec1Type&),
                _TargetType(const _Vec3Type&, const _Vec3Type&, const _Vec3Type&)
                >(),
                sol::meta_function::addition, make_vec_add<_TargetType, _NumType>(),
                sol::meta_function::subtraction, make_vec_sub<_TargetType, _NumType>(),
                sol::meta_function::multiplication, make_vec_mult<_TargetType, _NumType>(),
                sol::meta_function::division, make_mat_div<_TargetType, _NumType>()
            );
        }

        template<class _NumType>
        void register_mat4(const char* name, sol::state& lua)
        {
            using namespace glm;

            using _Vec4Type = vec<4, _NumType, defaultp>;
            using _TargetType = mat<4, 4, _NumType, defaultp>;

            lua.new_usertype<_TargetType>(
                name,
                sol::constructors<
                _TargetType(), _TargetType(_NumType), _TargetType(_NumType, _NumType, _NumType, _NumType, _NumType, _NumType, _NumType, _NumType, _NumType, _NumType, _NumType, _NumType, _NumType, _NumType, _NumType, _NumType),
                _TargetType(const _TargetType&),
                _TargetType(const _Vec4Type&, const _Vec4Type&, const _Vec4Type&, const _Vec4Type&)
                >(),
                sol::meta_function::addition, make_vec_add<_TargetType, _NumType>(),
                sol::meta_function::subtraction, make_vec_sub<_TargetType, _NumType>(),
                sol::meta_function::multiplication, make_vec_mult<_TargetType, _NumType>(),
                sol::meta_function::division, make_mat_div<_TargetType, _NumType>()
                );
        }

        template<class _NumType>
        void register_quat(const char* name, sol::state& lua)
        {
            using namespace glm;

            using _TargetType = qua<_NumType, defaultp>;
            using _Vec3Type = vec<3, _NumType, defaultp>;
            using _Matrix3Type = mat<3, 3, _NumType, defaultp>;
            using _Matrix4Type = mat<4, 4, _NumType, defaultp>;

            lua.new_usertype<_TargetType>(
                name,
                sol::constructors<
                _TargetType(), _TargetType(_NumType, _NumType, _NumType, _NumType),
                _TargetType(const _Vec3Type&, const _Vec3Type&), _TargetType(const _Vec3Type&),
                _TargetType(const _Matrix3Type&), _TargetType(const _Matrix4Type&)
                >(),
                sol::meta_function::addition, [](const _TargetType& a, const _TargetType& b) { return a + b; },
                sol::meta_function::subtraction, [](const _TargetType& a, const _TargetType& b) { return a - b; },
                sol::meta_function::multiplication, [](const _TargetType& a, const _TargetType& b) { return a * b; },
                sol::meta_function::equal_to, [](const _TargetType& a, const _TargetType& b) { return a == b; },
                "x", &_TargetType::x,
                "y", &_TargetType::y,
                "z", &_TargetType::z,
                "w", &_TargetType::w
            );
        }

		void bindGLMVector(sol::state* lua)
		{
            // Integer signed
            register_vec1<i32>("I32Vector1", *lua);
            register_vec2<i32>("I32Vector2", *lua);
            register_vec3<i32>("I32Vector3", *lua);
            register_vec4<i32>("I32Vector4", *lua);

            // UInteger unsigned
            register_vec1<u32>("U32Vector1", *lua);
            register_vec2<u32>("U32Vector2", *lua);
            register_vec3<u32>("U32Vector3", *lua);
            register_vec4<u32>("U32Vector4", *lua);
            
            // Floating point
            register_vec1<f32>("Vector1", *lua);
            register_vec2<f32>("Vector2", *lua);
            register_vec3<f32>("Vector3", *lua);
            register_vec4<f32>("Vector4", *lua);
            
            // Double
            register_vec1<f64>("F64Vector1", *lua);
            register_vec2<f64>("F64Vector2", *lua);
            register_vec3<f64>("F64Vector3", *lua);
            register_vec4<f64>("F64Vector4", *lua);
		}

		void bindGLMMatrix(sol::state* lua)
		{
            register_mat2<i32>("I32Matrix2", *lua);
            register_mat3<i32>("I32Matrix3", *lua);
            register_mat4<i32>("I32Matrix3", *lua);

            register_mat2<u32>("U32Matrix2", *lua);
            register_mat3<u32>("U32Matrix2", *lua);
            register_mat4<u32>("U32Matrix2", *lua);

            register_mat2<f32>("Matrix2", *lua);
            register_mat3<f32>("Matrix2", *lua);
            register_mat4<f32>("Matrix2", *lua);

            register_mat2<f64>("F64Matrix2", *lua);
            register_mat3<f64>("F64Matrix2", *lua);
            register_mat4<f64>("F64Matrix2", *lua);
		}

        void bingGLMGeometricFunctions(sol::state* lua)
        {
            lua->set_function("Length", sol::overload(
                [](glm::vec2& a) { return glm::length(a); },
                [](glm::vec3& a) { return glm::length(a); },
                [](glm::vec4& a) { return glm::length(a); },
                [](glm::dvec2& a) { return glm::length(a); },
                [](glm::dvec3& a) { return glm::length(a); },
                [](glm::dvec4& a) { return glm::length(a); }
            ));

            lua->set_function("Distance", sol::overload(
                [](glm::vec2& a, glm::vec2& b) { return glm::distance(a, b); },
                [](glm::vec3& a, glm::vec3& b) { return glm::distance(a, b); },
                [](glm::vec4& a, glm::vec4& b) { return glm::distance(a, b); },
                [](glm::dvec2& a, glm::dvec2& b) { return glm::distance(a, b); },
                [](glm::dvec3& a, glm::dvec3& b) { return glm::distance(a, b); },
                [](glm::dvec4& a, glm::dvec4& b) { return glm::distance(a, b); }
            ));

            lua->set_function("DotProduct", sol::overload(
                [](glm::vec2& a, glm::vec2& b) { return glm::dot(a, b); },
                [](glm::vec3& a, glm::vec3& b) { return glm::dot(a, b); },
                [](glm::vec4& a, glm::vec4& b) { return glm::dot(a, b); },
                [](glm::dvec2& a, glm::dvec2& b) { return glm::dot(a, b); },
                [](glm::dvec3& a, glm::dvec3& b) { return glm::dot(a, b); },
                [](glm::dvec4& a, glm::dvec4& b) { return glm::dot(a, b); }
            ));

            lua->set_function("CrossProduct", sol::overload(
                [](glm::vec3& a, glm::vec3& b) { return glm::cross(a, b); },
                [](glm::dvec3& a, glm::dvec3& b) { return glm::cross(a, b); }
            ));

            lua->set_function("Nrmalizeormalize", sol::overload(
                [](glm::vec2& a) { return glm::normalize(a); },
                [](glm::vec3& a) { return glm::normalize(a); },
                [](glm::vec4& a) { return glm::normalize(a); },
                [](glm::dvec2& a) { return glm::normalize(a); },
                [](glm::dvec3& a) { return glm::normalize(a); },
                [](glm::dvec4& a) { return glm::normalize(a); },
                [](glm::quat& a) { return glm::normalize(a); },
                [](glm::dquat& a) { return glm::normalize(a); }
            ));

            lua->set_function("Reflect", sol::overload(
                [](glm::vec2& a, glm::vec2& b) { return glm::reflect(a, b); },
                [](glm::vec3& a, glm::vec3& b) { return glm::reflect(a, b); },
                [](glm::vec4& a, glm::vec4& b) { return glm::reflect(a, b); },
                [](glm::dvec2& a, glm::dvec2& b) { return glm::reflect(a, b); },
                [](glm::dvec3& a, glm::dvec3& b) { return glm::reflect(a, b); },
                [](glm::dvec4& a, glm::dvec4& b) { return glm::reflect(a, b); }
            ));

            lua->set_function("Refract", sol::overload(
                [](glm::vec2& a, glm::vec2& b, f32 i) { return glm::refract(a, b, i); },
                [](glm::vec3& a, glm::vec3& b, f32 i) { return glm::refract(a, b, i); },
                [](glm::vec4& a, glm::vec4& b, f32 i) { return glm::refract(a, b, i); },
                [](glm::dvec2& a, glm::dvec2& b, f64 i) { return glm::refract(a, b, i); },
                [](glm::dvec3& a, glm::dvec3& b, f64 i) { return glm::refract(a, b, i); },
                [](glm::dvec4& a, glm::dvec4& b, f64 i) { return glm::refract(a, b, i); }
            ));
        }

        void bindGLMMatrixTransformation(sol::state* lua)
        {
            lua->set_function("TranslateMatrix", sol::overload(
                [](glm::mat4& a, glm::vec3& b) { return glm::translate(a, b); },
                [](glm::dmat4& a, glm::dvec3& b) { return glm::translate(a, b); }
                ));

            lua->set_function("RotateMatrix", sol::overload(
                [](glm::mat4& a, f32 b, glm::vec3& c) { return glm::rotate(a, b, c); },
                [](glm::dmat4& a, f64 b, glm::dvec3& c) { return glm::rotate(a, b, c); }
            ));
            
            lua->set_function("ScaleMatrix", sol::overload(
                [](glm::mat4& a, glm::vec3& b) { return glm::scale(a, b); },
                [](glm::dmat4& a, glm::dvec3& b) { return glm::scale(a, b); }
            ));

            lua->set_function("TransposeMatrix", sol::overload(
                [](glm::mat2& a) { return glm::transpose(a); },
                [](glm::mat3& a) { return glm::transpose(a); },
                [](glm::mat4& a) { return glm::transpose(a); },
                [](glm::dmat2& a) { return glm::transpose(a); },
                [](glm::dmat3& a) { return glm::transpose(a); },
                [](glm::dmat4& a) { return glm::transpose(a); }
            ));

            lua->set_function("InverseMatrix", sol::overload(
                [](glm::mat2& a) { return glm::inverse(a); },
                [](glm::mat3& a) { return glm::inverse(a); },
                [](glm::mat4& a) { return glm::inverse(a); },
                [](glm::dmat2& a) { return glm::inverse(a); },
                [](glm::dmat3& a) { return glm::inverse(a); },
                [](glm::dmat4& a) { return glm::inverse(a); }
            ));
        }
        
        void bindGLMQuaternion(sol::state* lua)
        {
            register_quat<f32>("Quaternion", *lua);
            register_quat<f64>("F64Quaternion", *lua);

            lua->set_function("QuatToEulerAngles", sol::overload(
                [](glm::quat& a) { return glm::eulerAngles(a); },
                [](glm::dquat& a) { return glm::eulerAngles(a); }
            ));

            lua->set_function("RollQuat", sol::overload(
                [](glm::quat& a) { return glm::roll(a); },
                [](glm::dquat& a) { return glm::roll(a); }
            ));

            lua->set_function("PitchQuat", sol::overload(
                [](glm::quat& a) { return glm::pitch(a); },
                [](glm::dquat& a) { return glm::pitch(a); }
            ));

            lua->set_function("YawQuat", sol::overload(
                [](glm::quat& a) { return glm::yaw(a); },
                [](glm::dquat& a) { return glm::yaw(a); }
            ));

            lua->set_function("QuatToMatrix3", sol::overload(
                [](glm::quat& a) { return glm::mat3_cast(a); },
                [](glm::dquat& a) { return glm::mat3_cast(a); }
            ));

            lua->set_function("QuatToMatrix4", sol::overload(
                [](glm::quat& a) { return glm::mat4_cast(a); },
                [](glm::dquat& a) { return glm::mat4_cast(a); }
            ));

            lua->set_function("MatrixToQuat", sol::overload(
                [](glm::mat3& a) { return glm::quat_cast(a); },
                [](glm::dmat3& a) { return glm::quat_cast(a); },
                [](glm::mat4& a) { return glm::quat_cast(a); },
                [](glm::dmat4& a) { return glm::quat_cast(a); }
            ));
        }

        void bindGLMCamera(sol::state* lua)
        {
            lua->set_function("PerspectiveProjection", sol::overload(
                [](f32 fov, f32 aspect, f32 znear, f32 zfar) { return glm::perspective(fov, aspect, znear, zfar); },
                [](f64 fov, f64 aspect, f64 znear, f64 zfar) { return glm::perspective(fov, aspect, znear, zfar); }
            ));
            
            lua->set_function("PerspectiveProjection", sol::overload(
                [](f32 left, f32 right, f32 bottom, f32 top) { return glm::ortho(left, right, bottom, top); },
                [](f64 left, f64 right, f64 bottom, f64 top) { return glm::ortho(left, right, bottom, top); }
            ));

            lua->set_function("LookAt", sol::overload(
                [](const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up) { return glm::lookAt(eye, center, up); },
                [](const glm::dvec3& eye, const glm::dvec3& center, const glm::dvec3& up) { return glm::lookAt(eye, center, up); }
            ));
        }

		void bindGLMath(sol::state* lua)
		{
			bindGLMVector(lua);
			bindGLMMatrix(lua);
            bingGLMGeometricFunctions(lua);
            bindGLMMatrixTransformation(lua);
            bindGLMQuaternion(lua);
            bindGLMCamera(lua);
		}
	}
}