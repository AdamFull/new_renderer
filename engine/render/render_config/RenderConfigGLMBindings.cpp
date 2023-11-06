#include "RenderConfigBindings.h"

#include <glm/glm.hpp>
#include <sol/sol.hpp>

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

        template<class _NumType>
        void register_vec1(const char* name, sol::state& lua)
        {
            using namespace glm;
            using _TargetType = vec<1, _NumType, defaultp>;

            lua.new_usertype<_TargetType>(
                name,
                sol::constructors<
                _TargetType(), _TargetType(_NumType),
                _TargetType(_TargetType)
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
                _TargetType(_TargetType), _TargetType(_Vec1Type), _TargetType(_Vec1Type, _Vec1Type),
                _TargetType(_Vec1Type, _NumType), _TargetType(_NumType, _Vec1Type)
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
                _TargetType(_TargetType), _TargetType(_Vec1Type), _TargetType(_Vec1Type, _Vec1Type, _Vec1Type),
                _TargetType(_Vec2Type, _NumType), _TargetType(_NumType, _Vec2Type)
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
                _TargetType(_TargetType), _TargetType(_Vec1Type), _TargetType(_Vec1Type, _Vec1Type, _Vec1Type, _Vec1Type),
                _TargetType(_Vec2Type, _NumType, _NumType), _TargetType(_NumType, _Vec2Type, _NumType), _TargetType(_NumType, _NumType, _Vec2Type), _TargetType(_Vec2Type, _Vec2Type),
                _TargetType(_Vec3Type, _NumType), _TargetType(_NumType, _Vec3Type)
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

        template<class _TargetType, class _Vec2Type, class _NumType>
        void register_mat2(const char* name, sol::state& lua)
        {
            //lua->new_usertype<glm::mat2>(
            //    "mat2",
            //    sol::constructors<
            //    glm::mat2(), glm::mat2(float), glm::mat2(float, float, float, float),
            //    glm::mat2(glm::mat2),
            //    glm::mat2(glm::vec2, glm::vec2)
            //    >(),
            //    sol::meta_function::addition, make_vec_add<glm::mat2, float>(),
            //    sol::meta_function::subtraction, make_vec_sub<glm::mat2, float>(),
            //    sol::meta_function::multiplication, make_vec_mult<glm::mat2, float>(),
            //    sol::meta_function::division, make_vec_div<glm::mat2, float>()
            //);
        }

		void bindGLMVector(sol::state* lua)
		{
            // Integer signed
            register_vec1<int32_t>("ivec1", *lua);
            register_vec2<int32_t>("ivec2", *lua);
            register_vec3<int32_t>("ivec3", *lua);
            register_vec4<int32_t>("ivec4", *lua);

            // UInteger unsigned
            register_vec1<uint32_t>("uvec1", *lua);
            register_vec2<uint32_t>("uvec2", *lua);
            register_vec3<uint32_t>("uvec3", *lua);
            register_vec4<uint32_t>("uvec4", *lua);
            
            // Floating point
            register_vec1<float>("vec1", *lua);
            register_vec2<float>("vec2", *lua);
            register_vec3<float>("vec3", *lua);
            register_vec4<float>("vec4", *lua);
            
            // Double
            register_vec1<double>("dvec1", *lua);
            register_vec2<double>("dvec2", *lua);
            register_vec3<double>("dvec3", *lua);
            register_vec4<double>("dvec4", *lua);
		}

		void bindGLMMatrix(sol::state* lua)
		{
            lua->new_usertype<glm::mat2>(
                "mat2",
                sol::constructors<
                glm::mat2(), glm::mat2(float), glm::mat2(float, float, float, float),
                glm::mat2(glm::mat2),
                glm::mat2(glm::vec2, glm::vec2)
                >(),
                sol::meta_function::addition, make_vec_add<glm::mat2, float>(),
                sol::meta_function::subtraction, make_vec_sub<glm::mat2, float>(),
                sol::meta_function::multiplication, make_vec_mult<glm::mat2, float>(),
                sol::meta_function::division, make_vec_div<glm::mat2, float>()
                );
		}

        void bingGLMFunctions(sol::state* lua)
        {

        }

		void bindGLMath(sol::state* lua)
		{
			bindGLMVector(lua);
			bindGLMMatrix(lua);
            bingGLMFunctions(lua);
		}
	}
}