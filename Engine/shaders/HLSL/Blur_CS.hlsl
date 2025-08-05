#include "Common.hlsli"

#define BLOCK_SIZE 16

struct BlurResourceHandles
{
    int blurTargetIdx;
    int renderTargetIdx;
    int kernelConstIdx;
};

struct Kernel
{
    uint nTaps;
    float coefficients[15];
};

ConstantBuffer<BlurResourceHandles> blurResourceHandles : register(b1);

struct CSInput
{
    uint3 GroupId : SV_GroupID;
    uint3 GroupThreadId : SV_GroupThreadID;
    uint3 DispatchThreadId : SV_DispatchThreadID;
    uint GroupIndex : SV_GroupIndex;
};

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void main(CSInput input)
{    
    Texture2D<float4> inTex = ResourceDescriptorHeap[blurResourceHandles.blurTargetIdx];
    RWTexture2D<float4> outTex = ResourceDescriptorHeap[blurResourceHandles.renderTargetIdx];
    ConstantBuffer<Kernel> kernel = ResourceDescriptorHeap[blurResourceHandles.kernelConstIdx];
        
    float2 uv = ((float2) input.DispatchThreadId.xy + 0.5f) * 1.0f / frameData.resolution;
        
    float width = frameData.resolution.x;
    float height = frameData.resolution.y;
    
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
            const float4 s = inTex.Sample(LinearMirrorSampler, tc).rgba;
            const float coef = kernel.coefficients[abs(x)] * kernel.coefficients[abs(y)];
            accAlpha += s.a * coef;
            maxColor = max(s.rgb, maxColor);
        }
    }
    
    outTex[input.DispatchThreadId.xy] = float4(maxColor, accAlpha);
}