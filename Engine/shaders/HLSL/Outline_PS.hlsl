#include "Common.hlsli"
#include "Scene.hlsli"

struct ImportMatCB
{
    int diffuseIdx;
    int normalIdx;
    int roughnessMetallicIdx;
    int materialConstIdx;
    int solidConstIdx;
};

struct SurfaceProps
{
    float3 materialColor;
};

float4 main() : SV_Target
{
    ConstantBuffer<ImportMatCB> importCB = ResourceDescriptorHeap[meshConstants.materialIdx];
    ConstantBuffer<SurfaceProps> surfaceProps = ResourceDescriptorHeap[importCB.solidConstIdx];
    
    return float4(surfaceProps.materialColor, 1.0f);
}