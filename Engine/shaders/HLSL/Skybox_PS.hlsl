SamplerState samplerState : register(s0);
TextureCube tex : register(t0);

float4 main(float3 worldPos : Position) : SV_TARGET
{
    //TextureCube tex = ResourceDescriptorHeap[0];
    return tex.Sample(samplerState, worldPos);
}