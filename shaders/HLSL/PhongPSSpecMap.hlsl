cbuffer LightCBuf : register(b1)
{
    float3 lightPos;
    float3 ambient;
    float3 diffuseColor;
    float diffuseIntensity;
    float attConst;
    float attLin;
    float attQuad;
};

cbuffer ObjectCBuf : register(b2)
{
    float3 specularIntensity;
    float specularPower;
    float padding[2];
};

Texture2D<float4> tex[2] : register(t0);
SamplerState samp;

float4 PSMain(float3 viewPos : POSITION, float3 n : NORMAL, float2 uv : TEXCOORD) : SV_TARGET
{
	// fragment to light vector data
    const float3 vToL = lightPos - viewPos;
    const float distToL = length(vToL);
    const float3 dirToL = vToL / distToL;
	// attenuation
    const float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
	// diffuse intensity
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, n));
	// reflected light vector
    const float3 w = n * dot(vToL, n);
    const float3 r = w * 2.0f - vToL;
	// calculate specular intensity based on angle between viewing vector and reflection vector, narrow with power function
    const float4 specularSample = tex[1].Sample(samp, uv);
    const float3 specularReflectionColor = specularSample.rgb;
    const float specPower = pow(2.0f, specularSample.a * 13.0f);
    const float3 specular = att * (diffuseColor * diffuseIntensity) * pow(max(0.0f, dot(normalize(-r), normalize(viewPos))), specPower);
	// final color
    return float4(saturate((diffuse + ambient) * tex[0].Sample(samp, uv).rgb + specular * specularReflectionColor), 1.0f);
}