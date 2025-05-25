float2 EncodeNormalOctahedron(float3 n)
{
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    n.xy = n.z >= 0.0 ? n.xy : (1.0 - abs(n.yx)) * lerp(-1.0, 1.0, n.xy >= 0.0);
    return n.xy;
}

float3 DecodeNormalOctahedron(float2 f)
{
    float3 n = float3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
    float t = saturate(-n.z);
    n.xy += lerp(t, -t, n.xy >= 0.0);
    return normalize(n);
}

float4 EncodeGBufferNormalRT(float3 viewNormal, float metallic, uint shadingExtension)
{
    float2 encodedNormal = EncodeNormalOctahedron(viewNormal) * 0.5f + 0.5f;
    return float4(encodedNormal, metallic, (float) shadingExtension / 255.0f);
}

void DecodeGBufferNormalRT(float4 data, out float3 viewNormal, out float metallic, out uint shadingExtension)
{
    viewNormal = DecodeNormalOctahedron(data.xy * 2.0f - 1.0f);
    metallic = data.z;
    shadingExtension = uint(data.w * 255.0f);
}