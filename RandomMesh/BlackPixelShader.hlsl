cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float3 light;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 normal : TEXCOORD0;
	float3 color : COLOR0;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	return float4(0.0f, 0.0f, 0.0f, 1.0f);
}