#pragma once

#include <cstdint>
#include <algorithm>

namespace ImDbg
{
    struct Vec2 { int16_t x, y; };

    inline Vec2 Min(const Vec2& a, const Vec2& b)
    { return { std::min(a.x, b.x), std::min(a.y, b.y) }; }

    inline Vec2 Max(const Vec2& a, const Vec2& b)
    { return { std::max(a.x, b.x), std::max(a.y, b.y) }; }

    Vec2 operator+(const Vec2& a, const Vec2& b) { return { int16_t(a.x + b.x), int16_t(a.y + b.y) }; }
    Vec2 operator-(const Vec2& a, const Vec2& b) { return { int16_t(a.x - b.x), int16_t(a.y - b.y) }; }
    Vec2 operator*(const Vec2& a, int16_t scalar) { return { int16_t(a.x * scalar), int16_t(a.y * scalar) }; }
    Vec2 operator/(const Vec2& a, int16_t scalar) { return { int16_t(a.x / scalar), int16_t(a.y / scalar) }; }


    struct Vec3 { int16_t x, y, z; };

    inline Vec3 Min(const Vec3& a, const Vec3& b)
    { return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) }; }

    inline Vec3 Max(const Vec3& a, const Vec3& b)
    { return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) }; }

    Vec3 operator+(const Vec3& a, const Vec3& b) { return { int16_t(a.x + b.x), int16_t(a.y + b.y), int16_t(a.z + b.z) }; }
    Vec3 operator-(const Vec3& a, const Vec3& b) { return { int16_t(a.x - b.x), int16_t(a.y - b.y), int16_t(a.z - b.z) }; }
    Vec3 operator*(const Vec3& a, int16_t scalar) { return { int16_t(a.x * scalar), int16_t(a.y * scalar), int16_t(a.z * scalar) }; }
    Vec3 operator/(const Vec3& a, int16_t scalar) { return { int16_t(a.x / scalar), int16_t(a.y / scalar), int16_t(a.z / scalar) }; }


    struct Vec4 { int16_t x, y, z, w; };

    inline Vec4 Min(const Vec4& a, const Vec4& b)
    { return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w) }; }

    inline Vec4 Max(const Vec4& a, const Vec4& b)
    { return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w) }; }

    Vec4 operator+(const Vec4& a, const Vec4& b) { return { int16_t(a.x + b.x), int16_t(a.y + b.y), int16_t(a.z + b.z), int16_t(a.w + b.w) }; }
    Vec4 operator-(const Vec4& a, const Vec4& b) { return { int16_t(a.x - b.x), int16_t(a.y - b.y), int16_t(a.z - b.z), int16_t(a.w - b.w) }; }
    Vec4 operator*(const Vec4& a, int16_t scalar) { return { int16_t(a.x * scalar), int16_t(a.y * scalar), int16_t(a.z * scalar), int16_t(a.w * scalar) }; }
    Vec4 operator/(const Vec4& a, int16_t scalar) { return { int16_t(a.x / scalar), int16_t(a.y / scalar), int16_t(a.z / scalar), int16_t(a.w / scalar) }; }


    struct UVec2 { uint16_t x, y; };

    inline UVec2 Min(const UVec2& a, const UVec2& b)
    { return { std::min(a.x, b.x), std::min(a.y, b.y) }; }

    inline UVec2 Max(const UVec2& a, const UVec2& b)
    { return { std::max(a.x, b.x), std::max(a.y, b.y) }; }

    UVec2 operator+(const UVec2& a, const UVec2& b) { return { uint16_t(a.x + b.x), uint16_t(a.y + b.y) }; }
    UVec2 operator-(const UVec2& a, const UVec2& b) { return { uint16_t(a.x - b.x), uint16_t(a.y - b.y) }; }
    UVec2 operator*(const UVec2& a, uint16_t scalar) { return { uint16_t(a.x * scalar), uint16_t(a.y * scalar) }; }
    UVec2 operator/(const UVec2& a, uint16_t scalar) { return { uint16_t(a.x / scalar), uint16_t(a.y / scalar) }; }


    struct UVec3 { uint16_t x, y, z; };

    inline UVec3 Min(const UVec3& a, const UVec3& b)
    { return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) }; }

    inline UVec3 Max(const UVec3& a, const UVec3& b)
    { return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) }; }

    UVec3 operator+(const UVec3& a, const UVec3& b) { return { uint16_t(a.x + b.x), uint16_t(a.y + b.y), uint16_t(a.z + b.z) }; }
    UVec3 operator-(const UVec3& a, const UVec3& b) { return { uint16_t(a.x - b.x), uint16_t(a.y - b.y), uint16_t(a.z - b.z) }; }
    UVec3 operator*(const UVec3& a, uint16_t scalar) { return { uint16_t(a.x * scalar), uint16_t(a.y * scalar), uint16_t(a.z * scalar) }; }
    UVec3 operator/(const UVec3& a, uint16_t scalar) { return { uint16_t(a.x / scalar), uint16_t(a.y / scalar), uint16_t(a.z / scalar) }; }


    struct UVec4 { uint16_t x, y, z, w; };

    inline UVec4 Min(const UVec4& a, const UVec4& b)
    { return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w) }; }

    inline UVec4 Max(const UVec4& a, const UVec4& b)
    { return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w) }; }

    UVec4 operator+(const UVec4& a, const UVec4& b) { return { uint16_t(a.x + b.x), uint16_t(a.y + b.y), uint16_t(a.z + b.z), uint16_t(a.w + b.w) }; }
    UVec4 operator-(const UVec4& a, const UVec4& b) { return { uint16_t(a.x - b.x), uint16_t(a.y - b.y), uint16_t(a.z - b.z), uint16_t(a.w - b.w) }; }
    UVec4 operator*(const UVec4& a, uint16_t scalar) { return { uint16_t(a.x * scalar), uint16_t(a.y * scalar), uint16_t(a.z * scalar), uint16_t(a.w * scalar) }; }
    UVec4 operator/(const UVec4& a, uint16_t scalar) { return { uint16_t(a.x / scalar), uint16_t(a.y / scalar), uint16_t(a.z / scalar), uint16_t(a.w / scalar) }; }



    inline Vec2 ToVec2(const UVec2& u) { return { int16_t(u.x), int16_t(u.y) }; }
    inline UVec2 ToUVec2(const Vec2& v) { return { uint16_t(v.x), uint16_t(v.y) }; }

    inline Vec3 ToVec3(const UVec3& u) { return { int16_t(u.x), int16_t(u.y), int16_t(u.z) }; }
    inline UVec3 ToUVec3(const Vec3& v) { return { uint16_t(v.x), uint16_t(v.y), uint16_t(v.z) }; }

    inline Vec4 ToVec4(const UVec4& u) { return { int16_t(u.x), int16_t(u.y), int16_t(u.z), int16_t(u.w) }; }
    inline UVec4 ToUVec4(const Vec4& v) { return { uint16_t(v.x), uint16_t(v.y), uint16_t(v.z), uint16_t(v.w) }; }
}