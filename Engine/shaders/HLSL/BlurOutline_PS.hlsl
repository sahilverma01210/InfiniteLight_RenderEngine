struct Kernel
{
    uint nTaps;
    float coefficients[15];
};

ConstantBuffer<Kernel> kernel : register(b0);
bool horizontal : register(b1);

Texture2D tex : register(t0);

float4 main(float2 uv : Texcoord) : SV_Target
{
    SamplerState samplerState = SamplerDescriptorHeap[0];
    
    float width, height;
    tex.GetDimensions(width, height);
    float dx, dy;
    if (horizontal)
    {
        dx = 1.0f / width;
        dy = 0.0f;
    }
    else
    {
        dx = 0.0f;
        dy = 1.0f / height;
    }
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