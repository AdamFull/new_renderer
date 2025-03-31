RWTexture2D<float4> write_image0;// : register(u0, space0);
RWTexture2D<float4> write_image1;// : register(u1, space0);
RWTexture2D<float4> write_image2;// : register(u2, space0);
RWTexture2D<float4> write_image3;// : register(u3, space0);

Sampler2D<float4> sample_image0;// : register(t0, space0);
//Texture2D<float4> sample_image0 : register(t0, space0);
//SamplerState image_sampler0 : register(s0, space0);

// Shared memory for inter-thread communication
groupshared float color_r[64];
groupshared float color_g[64];
groupshared float color_b[64];
groupshared float color_a[64];

void StoreColor(uint index, float4 color)
{
    color_r[index] = color.r;
    color_g[index] = color.g;
    color_b[index] = color.b;
    color_a[index] = color.a;
}

float4 LoadColor(uint index)
{
    return float4(color_r[index], color_g[index], color_b[index], color_a[index]);
}

// #define MIP_2
// #define MIP_3
// #define MIP_4

[shader("compute")]
[numthreads(8, 8, 1)]
void main(uint3 DTid: SV_DispatchThreadID, uint3 GTid: SV_GroupThreadID, uint GIid: SV_GroupIndex)
{
    // Compute the pixel coordinates:
    int2 id = int2(DTid.xy);

    // Get output image dimensions:
    uint width, height;
    write_image0.GetDimensions(width, height);

    // Convert to float2 for texture sampling:
    float2 dims = float2(width, height);
    float2 texelSize = 1.0 / dims;

    // Compute UV in the same manner as the GLSL code:
    float2 uv = texelSize * (float2(id) + 0.5);

    // Sample from the input texture
    float4 color0 = sample_image0.Sample(uv);

    // Write out to MIP 1 (base-level)
    write_image0[id] = color0;

#ifdef MIP_2
    // Store in group-shared memory
    StoreColor(GIid, color0);

    // Barrier for group-shared writes
    GroupMemoryBarrierWithGroupSync();

    // If X and Y are even => (GIid & 0x9) == 0
    if ((GIid & 0x9) == 0)
    {
        float4 color1 = LoadColor(GIid + 0x01);
        float4 color2 = LoadColor(GIid + 0x08);
        float4 color3 = LoadColor(GIid + 0x09);

        color0 = 0.25 * (color0 + color1 + color2 + color3);

        // Write the 2x2 reduced color
        write_image1[id / 2] = color0;

        // Store again in shared memory, as the next pass depends on it
        StoreColor(GIid, color0);
    }

#endif

#ifdef MIP_3
    GroupMemoryBarrierWithGroupSync();

    // If X and Y are multiples of four => (GIid & 0x1B) == 0
    //   0x1B in binary is 11011, but the relevant bits are in the low 6 bits
    //   This check means GTid.x is multiple of 4 and GTid.y is multiple of 4.
    if ((GIid & 0x1B) == 0)
    {
        float4 color1 = LoadColor(GIid + 0x02);
        float4 color2 = LoadColor(GIid + 0x10);
        float4 color3 = LoadColor(GIid + 0x12);

        color0 = 0.25 * (color0 + color1 + color2 + color3);

        // Write the 4x4 reduced color
        write_image2[id / 4] = color0;

        StoreColor(GIid, color0);
    }
#endif

#ifdef MIP_4
    GroupMemoryBarrierWithGroupSync();

    // If X and Y are multiples of eight => only GIid == 0 in an 8x8 group
    if (GIid == 0)
    {
        float4 color1 = LoadColor(GIid + 0x04);
        float4 color2 = LoadColor(GIid + 0x20);
        float4 color3 = LoadColor(GIid + 0x24);

        color0 = 0.25 * (color0 + color1 + color2 + color3);

        // Write the 8x8 reduced color
        write_image3[id / 8] = color0;
    }
#endif
}