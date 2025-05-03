#pragma once

// FidelityFX Super Resolution Sample
//
// Copyright (c) 2021 Advanced Micro Devices, Inc. All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//------------------------------------------------------------------------------------------------------------------------------
// MIT LICENSE
// ===========
// Copyright (c) 2014 Michal Drobot (for concepts used in "FLOAT APPROXIMATIONS").
// -----------
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,//
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
// -----------
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
// Software.
// -----------
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

inline uint32_t ABfiM(uint32_t src, uint32_t ins, uint32_t bits) {
    uint32_t mask = (1 << bits) - 1;
    return (ins & mask) | (src & (~mask));
}

inline uint32_t ABfe(uint32_t src, uint32_t off, uint32_t bits) {
    uint32_t mask = (1 << bits) - 1;
    return (src >> off) & mask;
}

// Simple remap 64x1 to 8x8 with rotated 2x2 pixel quads in quad linear.
//  543210
//  ======
//  ..xxx.
//  yy...y
inline uint32_t2 ARmp8x8(uint32_t a) {
    return uint32_t2(ABfe(a, 1, 3), ABfiM(ABfe(a, 3, 3), a, 1));
}

// More complex remap 64x1 to 8x8 which is necessary for 2D wave reductions.
//  543210
//  ======
//  .xx..x
//  y..yy.
// Details,
//  LANE TO 8x8 MAPPING
//  ===================
//  00 01 08 09 10 11 18 19
//  02 03 0a 0b 12 13 1a 1b
//  04 05 0c 0d 14 15 1c 1d
//  06 07 0e 0f 16 17 1e 1f
//  20 21 28 29 30 31 38 39
//  22 23 2a 2b 32 33 3a 3b
//  24 25 2c 2d 34 35 3c 3d
//  26 27 2e 2f 36 37 3e 3f
inline uint32_t2 ARmpRed8x8(uint32_t a) {
    return uint32_t2(ABfiM(ABfe(a, 2u, 3u), a, 1u), ABfiM(ABfe(a, 3u, 3u), ABfe(a, 1u, 2u), 2u));
}

// Remap 256x1 to 16x16 with rotated 2x2 pixel quads in quad linear.
//  76543210
//  ========
//  ...xxxx.
//  yyy...yy
inline uint32_t2 ARmp16x16(uint32_t a) {
    return uint32_t2(ABfe(a, 1, 4), ABfiM(ABfe(a, 4, 4), a, 1));
}

// More complex remap 256x1 to 16x16 which is necessary for 2D wave reductions.
//  76543210
//  ========
//  ..xxx..x
//  y...yyy.
// Details,
//  LANE TO 16x16 MAPPING
//  ====================
//  000 001 010 011 ... 0f0 0f1
//  002 003 012 013 ... 0f2 0f3
//  ...
//  00e 00f 01e 01f ... 0fe 0ff
//  020 021 030 031 ... 1f0 1f1
//  022 023 032 033 ... 1f2 1f3
//  ...
//  02e 02f 03e 03f ... 1fe 1ff
//  ...
//  ...
//  1e0 1e1 1f0 1f1 ... ff0 ff1
//  1e2 1e3 1f2 1f3 ... ff2 ff3
//  ...
//  1ee 1ef 1fe 1ff ... ffe fff
inline uint32_t2 ARmpRed16x16(uint32_t a) {
    return uint32_t2(ABfiM(ABfe(a, 2u, 4u), a, 1u), ABfiM(ABfe(a, 4u, 4u), ABfe(a, 1u, 2u), 3u));
}