#pragma once

#include "compute.nv.slang"
#include "compute.amd.slang"

#ifndef WG_TILE_SIZE
#define WG_TILE_SIZE 8
#endif

#define USE_AMD_APROACH

inline int32_t2 remap_wg_8x8(in uint32_t2 gid, in uint32_t2 lid, in uint32_t2 wid, in uint32_t2 face_size) {
#ifdef USE_AMD_APROACH
    uint32_t2 base_tx = gid - lid;
    uint lid1d = lid.y * 8 + lid.x;
    uint32_t2 sub_xy = ARmp8x8(lid1d);
    uint32_t2 tx = uint32_t2(sub_xy.x + 8 * ((lid1d >> 6) % 2), sub_xy.y + 8 * ((lid1d >> 7)));
    return int32_t2(base_tx + tx);
#else
    const uint32_t2 cta_dim = uint32_t2(8, 8);
    return int32_t2(thread_group_tiling_x(face_size / cta_dim, cta_dim, 32, lid, wid));
#endif
}

inline int32_t2 remap_wg_16x16(in uint32_t2 gid, in uint32_t2 lid, in uint32_t2 wid, in uint32_t2 face_size) {
#ifdef USE_AMD_APROACH
    uint32_t2 base_tx = gid - lid;
    uint lid1d = lid.y * 16 + lid.x;
    uint32_t2 sub_xy = ARmp16x16(lid1d);
    uint32_t2 tx = uint32_t2(sub_xy.x + 16 * ((lid1d >> 8) % 2), sub_xy.y + 16 * ((lid1d >> 9)));
    return int32_t2(base_tx + tx);
#else
    const uint32_t2 cta_dim = uint32_t2(16, 16);
    return int32_t2(thread_group_tiling_x(face_size / cta_dim, cta_dim, 16, lid, wid));
#endif
}

inline bool swizzle_reject_workgroups(in uint32_t2 gid, in uint32_t2 lid, in uint32_t2 wid, in uint32_t2 face_size, out float2 uv, out int32_t2 px) {
#if WG_TILE_SIZE == 8
    px = remap_wg_8x8(gid, lid, wid, face_size);
#elif WG_TILE_SIZE == 16
    px = remap_wg_16x16(gid, lid, wid, face_size);
#else
    px = gid;
#endif

    uv = (float2(px) + float2(0.5, 0.5)) / float2(face_size);
    if (px.x > face_size.x || px.y > face_size.y) {
        return true;
    }

    return false;
}