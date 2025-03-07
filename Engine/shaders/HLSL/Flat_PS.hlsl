#include "CommonResources.hlsl"

struct FlatCB
{
    float3 solidConstIdx;
};

struct PhongCB
{
    int shadowConstIdx;
    int lightConstIdx;
    int texConstIdx;
    int shadowTexIdx;
    int diffTexIdx;
    int normTexIdx;
    int specTexIdx;
    int solidConstIdx;
};

struct MaterialProps
{
    float3 materialColor;
};

float4 main() : SV_Target
{
    //ConstantBuffer<FlatCB> flatCB = ResourceDescriptorHeap[meshConstants.materialIdx];
    //ConstantBuffer<MaterialProps> materialProps = ResourceDescriptorHeap[flatCB.solidConstIdx];
    
    return float4(1.0f, 1.0f, 1.0f, 1.0f);;
}