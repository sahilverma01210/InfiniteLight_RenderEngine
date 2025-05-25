#include "Common.hlsli"
#include "Scene.hlsli"

struct MaterialCB
{
    int solidConstIdx;
};

struct SurfaceProps
{
    float3 materialColor;
};

float4 main() : SV_Target
{
    ConstantBuffer<MaterialCB> importCB = ResourceDescriptorHeap[meshConstants.materialIdx];
    ConstantBuffer<SurfaceProps> surfaceProps = ResourceDescriptorHeap[importCB.solidConstIdx];
    
    return float4(surfaceProps.materialColor, 1.0f);
}