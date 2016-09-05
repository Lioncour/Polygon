cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float3 light;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 normal : TEXCOORD0;
	float3 color : COLOR0;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float value = dot(normalize(light), normalize(input.normal));

	float4 finalColor = float4(input.color, 1.0f) * 0.5f + saturate(value * float4(input.color, 1.0f)) * 0.5f;
	finalColor.a = 1;

	return finalColor;
}
