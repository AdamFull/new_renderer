#pragma once

#include <cstdint>
#include <algorithm>

namespace ImDbg
{
    struct IVec2 { int16_t x, y; };

    inline IVec2 Min(const IVec2& a, const IVec2& b)
    { return { std::min(a.x, b.x), std::min(a.y, b.y) }; }

    inline IVec2 Max(const IVec2& a, const IVec2& b)
    { return { std::max(a.x, b.x), std::max(a.y, b.y) }; }

    inline IVec2 operator+(const IVec2& a, const IVec2& b) { return { int16_t(a.x + b.x), int16_t(a.y + b.y) }; }
    inline IVec2 operator-(const IVec2& a, const IVec2& b) { return { int16_t(a.x - b.x), int16_t(a.y - b.y) }; }
    inline IVec2 operator*(const IVec2& a, int16_t scalar) { return { int16_t(a.x * scalar), int16_t(a.y * scalar) }; }
    inline IVec2 operator/(const IVec2& a, int16_t scalar) { return { int16_t(a.x / scalar), int16_t(a.y / scalar) }; }


    struct IVec3 { int16_t x, y, z; };

    inline IVec3 Min(const IVec3& a, const IVec3& b)
    { return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) }; }

    inline IVec3 Max(const IVec3& a, const IVec3& b)
    { return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) }; }

    inline IVec3 operator+(const IVec3& a, const IVec3& b) { return { int16_t(a.x + b.x), int16_t(a.y + b.y), int16_t(a.z + b.z) }; }
    inline IVec3 operator-(const IVec3& a, const IVec3& b) { return { int16_t(a.x - b.x), int16_t(a.y - b.y), int16_t(a.z - b.z) }; }
    inline IVec3 operator*(const IVec3& a, int16_t scalar) { return { int16_t(a.x * scalar), int16_t(a.y * scalar), int16_t(a.z * scalar) }; }
    inline IVec3 operator/(const IVec3& a, int16_t scalar) { return { int16_t(a.x / scalar), int16_t(a.y / scalar), int16_t(a.z / scalar) }; }


    struct IVec4 { int16_t x, y, z, w; };

    inline IVec4 Min(const IVec4& a, const IVec4& b)
    { return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w) }; }

    inline IVec4 Max(const IVec4& a, const IVec4& b)
    { return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w) }; }

    inline IVec4 operator+(const IVec4& a, const IVec4& b) { return { int16_t(a.x + b.x), int16_t(a.y + b.y), int16_t(a.z + b.z), int16_t(a.w + b.w) }; }
    inline IVec4 operator-(const IVec4& a, const IVec4& b) { return { int16_t(a.x - b.x), int16_t(a.y - b.y), int16_t(a.z - b.z), int16_t(a.w - b.w) }; }
    inline IVec4 operator*(const IVec4& a, int16_t scalar) { return { int16_t(a.x * scalar), int16_t(a.y * scalar), int16_t(a.z * scalar), int16_t(a.w * scalar) }; }
    inline IVec4 operator/(const IVec4& a, int16_t scalar) { return { int16_t(a.x / scalar), int16_t(a.y / scalar), int16_t(a.z / scalar), int16_t(a.w / scalar) }; }


    struct UVec2 { uint16_t x, y; };

    inline UVec2 Min(const UVec2& a, const UVec2& b)
    { return { std::min(a.x, b.x), std::min(a.y, b.y) }; }

    inline UVec2 Max(const UVec2& a, const UVec2& b)
    { return { std::max(a.x, b.x), std::max(a.y, b.y) }; }

    inline UVec2 operator+(const UVec2& a, const UVec2& b) { return { uint16_t(a.x + b.x), uint16_t(a.y + b.y) }; }
    inline UVec2 operator-(const UVec2& a, const UVec2& b) { return { uint16_t(a.x - b.x), uint16_t(a.y - b.y) }; }
    inline UVec2 operator*(const UVec2& a, uint16_t scalar) { return { uint16_t(a.x * scalar), uint16_t(a.y * scalar) }; }
    inline UVec2 operator/(const UVec2& a, uint16_t scalar) { return { uint16_t(a.x / scalar), uint16_t(a.y / scalar) }; }


    struct UVec3 { uint16_t x, y, z; };

    inline UVec3 Min(const UVec3& a, const UVec3& b)
    { return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) }; }

    inline UVec3 Max(const UVec3& a, const UVec3& b)
    { return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) }; }

    inline UVec3 operator+(const UVec3& a, const UVec3& b) { return { uint16_t(a.x + b.x), uint16_t(a.y + b.y), uint16_t(a.z + b.z) }; }
    inline UVec3 operator-(const UVec3& a, const UVec3& b) { return { uint16_t(a.x - b.x), uint16_t(a.y - b.y), uint16_t(a.z - b.z) }; }
    inline UVec3 operator*(const UVec3& a, uint16_t scalar) { return { uint16_t(a.x * scalar), uint16_t(a.y * scalar), uint16_t(a.z * scalar) }; }
    inline UVec3 operator/(const UVec3& a, uint16_t scalar) { return { uint16_t(a.x / scalar), uint16_t(a.y / scalar), uint16_t(a.z / scalar) }; }


    struct UVec4 { uint16_t x, y, z, w; };

    inline UVec4 Min(const UVec4& a, const UVec4& b)
    { return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w) }; }

    inline UVec4 Max(const UVec4& a, const UVec4& b)
    { return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w) }; }

    inline UVec4 operator+(const UVec4& a, const UVec4& b) { return { uint16_t(a.x + b.x), uint16_t(a.y + b.y), uint16_t(a.z + b.z), uint16_t(a.w + b.w) }; }
    inline UVec4 operator-(const UVec4& a, const UVec4& b) { return { uint16_t(a.x - b.x), uint16_t(a.y - b.y), uint16_t(a.z - b.z), uint16_t(a.w - b.w) }; }
    inline UVec4 operator*(const UVec4& a, uint16_t scalar) { return { uint16_t(a.x * scalar), uint16_t(a.y * scalar), uint16_t(a.z * scalar), uint16_t(a.w * scalar) }; }
    inline UVec4 operator/(const UVec4& a, uint16_t scalar) { return { uint16_t(a.x / scalar), uint16_t(a.y / scalar), uint16_t(a.z / scalar), uint16_t(a.w / scalar) }; }



    inline IVec2 ToVec2(const UVec2& u) { return { int16_t(u.x), int16_t(u.y) }; }
    inline UVec2 ToUVec2(const IVec2& v) { return { uint16_t(v.x), uint16_t(v.y) }; }

    inline IVec3 ToVec3(const UVec3& u) { return { int16_t(u.x), int16_t(u.y), int16_t(u.z) }; }
    inline UVec3 ToUVec3(const IVec3& v) { return { uint16_t(v.x), uint16_t(v.y), uint16_t(v.z) }; }

    inline IVec4 ToVec4(const UVec4& u) { return { int16_t(u.x), int16_t(u.y), int16_t(u.z), int16_t(u.w) }; }
    inline UVec4 ToUVec4(const IVec4& v) { return { uint16_t(v.x), uint16_t(v.y), uint16_t(v.z), uint16_t(v.w) }; }
}