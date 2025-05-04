#pragma once

namespace util {
    inline float2 uv_to_clip(in float2 uv) {
        return uv * 2.0 - 1.0;
    }

    inline float2 texel_2d_to_uv(uint32_t2 texel, float2 texel_size) {
        return ((float2)texel + 0.5f) * texel_size;
    }

    inline float3 texel_3d_to_uv(uint32_t3 texel, float3 texel_size) {
        return ((float3)texel + 0.5f) * texel_size;
    }
    
    inline float2 clip_to_uv(in float2 clip) {
        return clip * 0.5f + 0.5f;
    }

    inline float3 position_from_depth(float2 uv, float d, in float4x4 converter) {
        float4 clip = float4(uv_to_clip(uv), d, 1.0);
        float4 pos = mul(converter, clip);
        return float3(pos.xyz / pos.w);
    }

    inline float linearize_depth(in float z, in float n, in float f) {
        return (2.0f * n * f) / (f + n - z * (f - n));
    }

    float2 encode_velocity(in float2 velocity) {
#ifdef LOW_PRECISION_VELOCITY
        return pow(clip_to_uv(velocity), float2(3.0));
#else
        return clip_to_uv(velocity);
#endif
    }

    float2 decode_velocity(in float2 velocity) {
#ifdef LOW_PRECISION_VELOCITY
        return uv_to_clip(pow(velocity, float2(1.0 / 3.0)));
#else
        return uv_to_clip(velocity);
#endif
    }

    inline float2 barrel_distortion(float2 coord, float amt) {
        float2 cc = coord - 0.5;
        float dist = dot(cc, cc);
        return coord + cc * dist * amt;
    }

    inline float linterp(float t) {
        return saturate(1.0 - abs(2.0 * t - 1.0));
    }

    inline float remap(float t, float a, float b) {
        return saturate((t - a) / (b - a));
    }

    inline float4 spectrum_offset(float t) {
        float4 ret;
        float lo = step(t, 0.5);
        float hi = 1.0 - lo;
        float w = linterp(remap(t, 1.0 / 6.0, 5.0 / 6.0));
        ret = float4(lo, 1.0, hi, 1.) * float4(1.0 - w, w, 1.0 - w, 1.);

        return pow(ret, float4(1.0 / 2.2));
    }

    inline float3 convert_xyz_2_yxy(float3 _xyz) {
        // Reference(s):
        // - XYZ to xyY
        //   https://web.archive.org/web/20191027010144/http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_xyY.html
        float inv = 1.0 / dot(_xyz, float3(1.0, 1.0, 1.0));
        return float3(_xyz.y, _xyz.x * inv, _xyz.y * inv);
    }

    inline float3 convert_xyz_2_rgb(float3 _xyz) {
        float3 rgb;
        rgb.x = dot(float3(3.2404542, -1.5371385, -0.4985314), _xyz);
        rgb.y = dot(float3(-0.9692660, 1.8760108, 0.0415560), _xyz);
        rgb.z = dot(float3(0.0556434, -0.2040259, 1.0572252), _xyz);
        return rgb;
    }

    inline float3 convert_rgb_2_xyz(float3 _rgb) {
        // Reference(s):
        // - RGB/XYZ Matrices
        //   https://web.archive.org/web/20191027010220/http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
        float3 xyz;
        xyz.x = dot(float3(0.4124564, 0.3575761, 0.1804375), _rgb);
        xyz.y = dot(float3(0.2126729, 0.7151522, 0.0721750), _rgb);
        xyz.z = dot(float3(0.0193339, 0.1191920, 0.9503041), _rgb);
        return xyz;
    }

    inline float3 convert_yxy_2_xyz(float3 _Yxy) {
        // Reference(s):
        // - xyY to XYZ
        //   https://web.archive.org/web/20191027010036/http://www.brucelindbloom.com/index.html?Eqn_xyY_to_XYZ.html
        float3 xyz;
        xyz.x = _Yxy.x * _Yxy.y / _Yxy.z;
        xyz.y = _Yxy.x;
        xyz.z = _Yxy.x * (1.0 - _Yxy.y - _Yxy.z) / _Yxy.z;
        return xyz;
    }

    inline float3 convert_rgb_2_yxy(float3 _rgb) {
        return convert_xyz_2_yxy(convert_rgb_2_xyz(_rgb));
    }

    inline float3 convert_yxy_2_rgb(float3 _Yxy) {
        return convert_xyz_2_rgb(convert_yxy_2_xyz(_Yxy));
    }

    inline float reinhard2(float _x, float _whiteSqr) {
        return (_x * (1.0 + _x / _whiteSqr)) / (1.0 + _x);
    }

    inline float3 to_gamma(float3 _rgb, float gamma) {
        return pow(abs(_rgb), float3(1.0 / gamma));
    }
}