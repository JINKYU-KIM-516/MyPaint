// Shader.hlsl
// texture drawing shader for blackboard

Texture2D tex : register(t0);
SamplerState samp : register(s0);

struct VS_INPUT
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD0;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = float4(input.pos, 1.0f);
    output.uv = input.uv;
    return output;
}

float4 PS(PS_INPUT input) : SV_TARGET
{
    return tex.Sample(samp, input.uv);
}
