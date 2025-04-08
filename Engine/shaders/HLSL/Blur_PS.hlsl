#include "CommonResources.hlsli"

struct PostProcessCB
{
    int kernelConstIdx;
    int frameBufferIdx;
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
    ConstantBuffer<Kernel> kernel = ResourceDescriptorHeap[postProcessCB.kernelConstIdx];    
    Texture2D<float4> tex = ResourceDescriptorHeap[postProcessCB.frameBufferIdx];
    
    float width, height;
    tex.GetDimensions(width, height);
    float dx, dy;
    
    dx = 1.0f / width;
    dy = 1.0f / height;
    
    const int r = kernel.nTaps / 2;
    
    float accAlpha = 0.0f;
    float3 maxColor = float3(0.0f, 0.0f, 0.0f);
    
    for (int y = -r; y <= r; y++)
    {
        for (int x = -r; x <= r; x++)
        {
            const float2 tc = uv + float2(dx * x, dy * y);
            const float4 s = tex.Sample(samplerState, tc).rgba;
            const float coef = kernel.coefficients[abs(x)] * kernel.coefficients[abs(y)];
            accAlpha += s.a * coef;
            maxColor = max(s.rgb, maxColor);
        }
    }
    return float4(maxColor, accAlpha);
}