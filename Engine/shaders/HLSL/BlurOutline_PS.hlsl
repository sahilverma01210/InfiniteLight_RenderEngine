#include "CommonResources.hlsl"

struct PostProcessCB
{
    int kernelConstIdx;
    int frameBufferIdx1;
    int frameBufferIdx2;
};

struct Kernel
{
    uint nTaps;
    float coefficients[15];
};

SamplerState samplerState : register(s0);

float4 main(float2 uv : Texcoord) : SV_Target
{
    ConstantBuffer<PostProcessCB> postProcessCB = ResourceDescriptorHeap[meshConstants.materialIdx];
    
    float width, height;
#if defined(HORIZONTAL_BLUR)
    Texture2D<float4> tex = ResourceDescriptorHeap[postProcessCB.frameBufferIdx1];
#else
    Texture2D<float4> tex = ResourceDescriptorHeap[postProcessCB.frameBufferIdx2];
#endif
    tex.GetDimensions(width, height);
    float dx, dy;
#if defined(HORIZONTAL_BLUR)
    dx = 1.0f / width;
    dy = 0.0f;
#else
    dx = 0.0f;
    dy = 1.0f / height;
#endif
    ConstantBuffer<Kernel> kernel = ResourceDescriptorHeap[postProcessCB.kernelConstIdx];
    
    const int r = kernel.nTaps / 2;
    
    float accAlpha = 0.0f;
    float3 maxColor = float3(0.0f, 0.0f, 0.0f);
    for (int i = -r; i <= r; i++)
    {
        const float2 tc = uv + float2(dx * i, dy * i);
        const float4 s = tex.Sample(samplerState, tc).rgba;
        const float coef = kernel.coefficients[i + r];
        accAlpha += s.a * coef;
        maxColor = max(s.rgb, maxColor);
    }
    return float4(maxColor, accAlpha);
}