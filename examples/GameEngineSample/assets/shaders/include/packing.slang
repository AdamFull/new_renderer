#pragma once

namespace rgba8_unorm {
	static const float val_scale = 255.0f;
	static const float inv_val_scale = 1.0f / val_scale;

    uint32_t pack(float4 value) {
        uint4 packed = uint32_t4(round(saturate(value) * val_scale));
        return  ((packed.x & 0xFF) << 0) | ((packed.y & 0xFF) << 8) | ((packed.z & 0xFF) << 16) | ((packed.w & 0xFF) << 24);
    }

    float4 unpack(uint32_t value) {
        uint32_t4 v;
        v.x = (value << 24) >> 24;
        v.y = (value << 16) >> 24;
        v.z = (value << 8) >> 24;
        v.w = (value << 0) >> 24;
        return float4(v) * inv_val_scale;
	}
}