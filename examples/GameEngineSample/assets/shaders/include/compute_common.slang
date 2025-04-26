#pragma once

uint32_t ABfiM(uint32_t src, uint32_t ins, uint32_t bits)
{
    uint32_t mask = (1 << bits) - 1;
    return (ins & mask) | (src & (~mask));
}

uint32_t ABfe(uint32_t src, uint32_t off, uint32_t bits)
{
    uint32_t mask = (1 << bits) - 1;
    return (src >> off) & mask;
}

uint32_t2 ARmp8x8(uint32_t a)
{
    return uint32_t2(ABfe(a, 1, 3), ABfiM(ABfe(a, 3, 3), a, 1));
}

bool swizzle_workgroups_8x8(in uint32_t2 gid, in uint32_t2 lid, in uint32_t2 wid, in uint32_t2 face_size, out float2 uv, out int32_t2 px) {
    uint32_t2 base_tx = gid - lid;
    uint lid1d = lid.y * 8 + lid.x;
    uint32_t2 sub_xy = ARmp8x8(lid1d);
    uint32_t2 tx = uint32_t2(sub_xy.x + 8 * ((lid1d >> 6) % 2), sub_xy.y + 8 * ((lid1d >> 7)));
    px = int32_t2(base_tx + tx);

    uv = (float2(px) + float2(0.5, 0.5)) / float2(face_size);
    if (px.x > face_size.x || px.y > face_size.y) {
        return true;
    }

    return false;
}